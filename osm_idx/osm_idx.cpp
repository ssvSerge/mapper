#include <iostream>
#include <string>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <list>
#include <deque>
#include <sstream>
#include <iomanip>
#include <functional> 
#include <fstream>
#include <unordered_map>
#include <utility>

#include "..\_tools\XmlInspector.hpp"

#include "osm_idx_tool.h"

using namespace std;

#define LOG_UNKNOWN_NODE                        (0)

#define CFG_TEXT_AREA                           "Area"
#define CFG_TEXT_AREA_BUILDING                  "Building"
#define CFG_TEXT_AREA_ASPHALT                   "Asphalt"

typedef enum tag_xml_cmd_id {
    CMD_ID_UNKNOWN = 0,
    CMD_ID_IGNORE,
    CMD_ID_DELETE,
    CMD_ID_POSITION,
    CMD_ID_LANES,
    CMD_ID_NAME,
    CMD_ID_ROAD,
    CMD_ID_AREA
}   XML_CMD_ID;

#define  OBJ_ID_DEFAULT                          (0x000000000000ULL)
#define  OBJ_ID_IGNORE                           (0x000000000001ULL)
#define  OBJ_ID_DELETE                           (0x000000000002ULL)
#define  OBJ_ID_TRANSPORT                        (0x000000000004ULL)
#define  OBJ_ID_TRANSPORT_BUS                    (0x000000000008ULL)
#define  OBJ_ID_TRANSPORT_TRAM                   (0x000000000010ULL)
#define  OBJ_ID_TRANSPORT_TRAIN                  (0x000000000020ULL)
#define  OBJ_ID_TRANSPORT_TROLLEY                (0x000000000040ULL)
#define  OBJ_ID_TRANSPORT_TAXI                   (0x000000000080ULL)
#define  OBJ_ID_TRANSPORT_METRO_STATION          (0x000000000100ULL)
#define  OBJ_ID_TRANSPORT_METRO_ENTRANCE         (0x000000000200ULL)
#define  OBJ_ID_ROAD_MOTORWAY                    (0x000000000400ULL)
#define  OBJ_ID_ROAD_PRIMARY                     (0x000000000800ULL)
#define  OBJ_ID_ROAD_SECONDARY                   (0x000000001000ULL)
#define  OBJ_ID_ROAD_RESIDENTIAL                 (0x000000002000ULL)
#define  OBJ_ID_ROAD_STREET                      (0x000000004000ULL)
#define  OBJ_ID_ROAD_PATH                        (0x000000010000ULL)
#define  OBJ_ID_ROAD_METRO                       (0x000000020000ULL)
#define  OBJ_ID_AREA_BUILDING                    (0x000000040000ULL)
#define  OBJ_ID_AREA_ASPHALT                     (0x000000080000ULL)



class XmlexInfo {
    public:
        XmlexInfo () {
            clear ();
        }

        void clear () {
            cmd = CMD_ID_UNKNOWN;
            k   = "";
            v   = "";
            t   = OBJ_ID_DEFAULT;
            p   = "";
        }

    public:
        XML_CMD_ID  cmd;
        uint64_t    t;  // decoded param value "t"
        string      k;  // param "k"
        string      v;  // param "v"
        string      p;  // additional parameter "p"
};

typedef vector<XmlexInfo> CFG_LEX_LIST;

class XmlParamsList {

    public:
        XmlParamsList () {
            clear ();
        }

        void clear () {
            for ( int i = 0; i < ARRAY_SIZE (p); i++ ) {
                p [i] = "";
            }
            info.k = "";
            info.v = "";
        }

    public:
        string     p [32];
        XmlexInfo  info;
};

typedef struct tag_xml_mapper {
    string                  param;
    uint64_t                key;
} XML_MAPPER;

typedef struct tag_osm_param_tag {
    string                  k;
    string                  v;
}   OSM_PARAM_TAG;

typedef list<OSM_PARAM_TAG>  OSM_PARAMS_LIST;

typedef struct tag_osm_text_info {
    string                  lang;
    string                  text;
}   OSM_TEXT_INFO;

typedef vector<OSM_TEXT_INFO> OSM_TEXT_LISTS;

typedef vector<uint64_t>  OSM_REF_LIST;

typedef struct tag_osm_node_cache {
    uint64_t                type;               // Тип объекта
    double                  lat;                // Широта 
    double                  lon;                // Долгота
}   OSM_NODE_CACHE;

typedef struct tag_osm_data_cache {
    uint64_t                id;                 // id нода или дороги
    uint64_t                off;                // смещение в файле
}   OSM_DATA_CACHE;

typedef enum tag_osm_member_type {
    MEMBER_TYPE_NODE,
    MEMBER_TYPE_WAY,
    MEMBER_TYPE_REL
}   OSM_MEMBER_TYPE;

typedef enum tag_osm_member_role {
    MEMBER_ROLE_OUTER,
    MEMBER_ROLE_INNER,
    MEMBER_ROLE_AREA
}   OSM_MEMBER_ROLE;

typedef struct tag_osm_param_member {
    OSM_MEMBER_TYPE         type;
    uint64_t                id;
    OSM_MEMBER_ROLE         role;
}   OSM_PARAM_MEMBER;

typedef vector<OSM_PARAM_MEMBER>   OSM_MEMBERS_LIST;

typedef struct tag_geo_coord {
    double                  lat;
    double                  lon;
}   GEO_COORD;

typedef list<GEO_COORD>     GEO_CORD_LIST;

class OsmNameInfo {
    public:
    OsmNameInfo () {}

    public:
    string      lang;
    string      text;
};

class OsmNodeInfo {
    public:
    OsmNodeInfo () {
        type = 0;
        lat = GEO_COORDINATE_ERROR;
        lon = GEO_COORDINATE_ERROR;
        in_use = false;
    }

    public:
    double      lat;
    double      lon;
    uint64_t    type;
    bool        in_use;
};

class OsmWayInfo {

    public:
        OsmWayInfo () {
            clear ();
        }

        void clear () {
            type = 0;
            lanes = 1;
            nodes.clear ();
            in_use = false;
        }

    public:
        uint64_t                type;
        uint64_t                lanes;
        GEO_CORD_LIST           nodes;
        bool                    in_use;
};

class OsmAreaInfo {
    public:
        OsmAreaInfo () {
            clear ();
        }

        void clear () {
            outline.clear ();
            cut_off_list.clear ();
            names_list.clear ();
            type = OBJ_ID_DEFAULT;
        }

    public:
        GEO_CORD_LIST           outline;
        vector<GEO_CORD_LIST>   cut_off_list;
        vector<OsmNameInfo>     names_list;
        uint64_t                type;
};

class OsmObject {

    public:
        OsmObject () {
            Clear ();
        }

    public:
        void Clear () {
            used        = false;
            deleted     = false;
            node.lat    = GEO_COORDINATE_ERROR;
            node.lon    = GEO_COORDINATE_ERROR;
            node_name.clear ();
            params.clear ();
            names.clear ();
            refs.clear ();
            members.clear ();
        }

    public:
        string              node_name;          // XML имя объекта (node, way, relation, nd, tag, member)

    public:
        OSM_NODE_CACHE      node;               // Данные для записи в файл.
        OSM_PARAM_TAG       tmp_tag;            // текущий TAG.
        OSM_PARAM_MEMBER    tmp_member;         // текущий MEMBER.
        uint64_t            id;                 // Номер нода
        uint64_t            obj_ref;            // 

    public:
        bool                used;               // TRUE в том случае, если объект используется в карте
        bool                deleted;            // TRUE если объект необходимо удалить (опции proposed, completted)
        OSM_PARAMS_LIST     params;             // Список тэгов k="" и v=""
        OSM_TEXT_LISTS      names;              // Список имен
        OSM_REF_LIST        refs;               // 
        OSM_MEMBERS_LIST    members;            // 
        OsmWayInfo          way;                // 
};

typedef unordered_map<uint64_t, OsmNameInfo>  OsmNamesMap;

typedef unordered_map<uint64_t, OsmNodeInfo>  OsmNodeMap;

typedef unordered_map<uint64_t, OsmWayInfo>   OsmWaysMap;

typedef unordered_map<uint64_t, OsmAreaInfo>  OsmAreaMap;


CFG_LEX_LIST        g_lex_list_nodes;
CFG_LEX_LIST        g_lex_list_ways;
CFG_LEX_LIST        g_lex_list_rels;
string              g_osm_file_name;
deque<OsmObject>    g_xml_ctx;

OsmNamesMap         g_osm_node_names_map;
OsmNodeMap          g_osm_nodes_map;
OsmWaysMap          g_osm_ways_map;
OsmAreaMap          g_osm_area_map;


bool LexCmp (string xml_node, string cfg_param) {

    if ( xml_node.length () == 0 ) {
        RET_FALSE;
    }
    if ( cfg_param.length () == 0 ) {
        RET_FALSE;
    }

    if ( cfg_param == "*" ) {
        return true;
    }

    bool bIsTemplate = false;
    if ( cfg_param [cfg_param.length () - 1] == '*' ) {
        bIsTemplate = true;
        cfg_param.erase (cfg_param.length () - 1);
    }

    if ( cfg_param.length () > xml_node.length () ) {
        return false;
    }

    if ( cfg_param.length () == xml_node.length () ) {
        bool ret_val = cfg_param == xml_node;
        return ret_val;
    }

    if ( !bIsTemplate ) {
        return false;
    }

    string substr = xml_node.substr (0, cfg_param.length ());

    if ( substr != cfg_param ) {
        return false;
    }

    return true;
}

bool MapNodeTypes (XmlParamsList& params_list) {

    static const XML_MAPPER mapper[] = {
        { "Transport",      OBJ_ID_TRANSPORT },
        { "MetroEntrance",  OBJ_ID_TRANSPORT_METRO_ENTRANCE },
        { "MetroStation",   OBJ_ID_TRANSPORT_METRO_STATION},
        { "Bus",            OBJ_ID_TRANSPORT_BUS },
        { "Tram",           OBJ_ID_TRANSPORT_TRAM },
        { "Train",          OBJ_ID_TRANSPORT_TRAIN },
        { "Trolley",        OBJ_ID_TRANSPORT_TROLLEY },
        { "Taxi",           OBJ_ID_TRANSPORT_TAXI }
    };

    uint64_t t = OBJ_ID_DEFAULT;
    int i;
    int j;

    params_list.info.t = OBJ_ID_DEFAULT;

    for ( i = 0; i < ARRAY_SIZE (params_list.p); i++ ) {

        if ( params_list.p [i].empty () ) {
            break;
        }

        for ( j = 0; j < ARRAY_SIZE (mapper); j++ ) {
            if ( params_list.p [i] == mapper [j].param ) {
                t |= mapper [j].key;
                break;
            }
        }

        if ( j == ARRAY_SIZE (mapper) ) {
            RET_FALSE;
        }
    }

    params_list.info.t = t;

    return true;
}

bool MapWayTypes (XmlParamsList& params_list) {

    static const XML_MAPPER mapper[] = {
        { "Motorway",       OBJ_ID_ROAD_MOTORWAY },
        { "Primary",        OBJ_ID_ROAD_PRIMARY },
        { "Secondary",      OBJ_ID_ROAD_SECONDARY },
        { "Residential",    OBJ_ID_ROAD_RESIDENTIAL },
        { "Street",         OBJ_ID_ROAD_STREET },
        { "Path",           OBJ_ID_ROAD_PATH },
        { "Metro",          OBJ_ID_ROAD_METRO }
    };

    uint64_t t = OBJ_ID_DEFAULT;
    int i;
    int j;

    params_list.info.t = OBJ_ID_DEFAULT;

    for ( i = 0; i < ARRAY_SIZE (params_list.p); i++ ) {

        if ( params_list.p [i].empty () ) {
            break;
        }

        for ( j = 0; j < ARRAY_SIZE (mapper); j++ ) {
            if ( params_list.p [i] == mapper [j].param ) {
                t |= mapper [j].key;
                break;
            }
        }

        if ( j == ARRAY_SIZE (mapper) ) {
            RET_FALSE;
        }
    }

    params_list.info.t = t;

    return true;
}

bool MapAreaTypes (XmlParamsList& params_list) {

    static const XML_MAPPER mapper[] = {
        { "Building",      OBJ_ID_AREA_BUILDING },
        { "Asphalt",       OBJ_ID_AREA_ASPHALT }
    };

    uint64_t t = OBJ_ID_DEFAULT;
    int i;
    int j;

    params_list.info.t = OBJ_ID_DEFAULT;

    for ( i = 0; i < ARRAY_SIZE (params_list.p); i++ ) {

        if ( params_list.p [i].empty () ) {
            break;
        }

        for ( j = 0; j < ARRAY_SIZE (mapper); j++ ) {
            if ( params_list.p [i] == mapper [j].param ) {
                t |= mapper [j].key;
                break;
            }
        }

        if ( j == ARRAY_SIZE (mapper) ) {
            RET_FALSE;
        }
    }

    params_list.info.t = t;

    return true;
}

string& trim (string &s, string delims = " ") {

    size_t l, r, c;
    std::size_t found;

    for ( l = 0; l < s.length (); l++ ) {
        found = delims.find (s [l]);
        if ( found == string::npos ) {
            break;
        }
    }

    if ( l == s.length () ) {
        s = "";
        return s;
    }

    for ( r = s.length () - 1; r > l; r-- ) {
        found = delims.find (s [r]);
        if ( found == string::npos ) {
            break;
        }
    }

    c = (r - l + 1);
    s = s.substr (l, c);

    return s;
}

bool ProcessCfgLine (string str) {

    static const XML_MAPPER mapper[] = {
        { "Ignore",      CMD_ID_IGNORE },
        { "Delete",      CMD_ID_DELETE },
        { "Name",        CMD_ID_NAME },
        { "Position",    CMD_ID_POSITION },
        { "Area",        CMD_ID_AREA },
        { "Lanes",       CMD_ID_LANES },
        { "Road",        CMD_ID_ROAD }
    };

    int             cmd_id;
    XmlParamsList   xml_param;
    string          cmd_name;
    string          target_name;

    if ( str.length () <= 1 ) {
        return true;
    }

    if ( str [0] == '#' ) {
        return true;
    }

    stringstream ss (str);
    ss >> target_name;
    ss >> xml_param.info.k;
    ss >> xml_param.info.v;

    if ( xml_param.info.k.empty () || xml_param.info.v.empty () ) {
        RET_FALSE;
    }

    ss >> cmd_name;
    for ( cmd_id = 0; cmd_id < ARRAY_SIZE (mapper); cmd_id++ ) {
        if ( cmd_name == mapper [cmd_id].param ) {
            xml_param.info.cmd = (XML_CMD_ID) mapper [cmd_id].key;
            break;
        }
    }

    if ( cmd_id == ARRAY_SIZE (mapper) ) {
        RET_FALSE;
    }


    for ( int i = 0; i < ARRAY_SIZE (xml_param.p); i++ ) {
        ss >> xml_param.p [i];
        if ( xml_param.p [i].empty () ) {
            break;
        }
    }


    if ( target_name == "n" ) {

        if ( xml_param.info.cmd == CMD_ID_IGNORE ) {
            ;
        } else

        if ( xml_param.info.cmd == CMD_ID_DELETE ) {
            ;
        } else

        if ( xml_param.info.cmd == CMD_ID_NAME ) {
            if ( xml_param.p [0].empty () ) {
                RET_FALSE;
            }
            xml_param.info.t = CMD_ID_NAME;
            xml_param.info.p = xml_param.p [0];
        } else

        if ( xml_param.info.cmd == CMD_ID_POSITION ) {
            if ( ! MapNodeTypes (xml_param) ) {
                RET_FALSE;
            }
        } else

        {
            RET_FALSE;
        }

        g_lex_list_nodes.push_back (xml_param.info);

        return true;
    }

    if ( target_name == "w" ) {

        if ( xml_param.info.cmd == CMD_ID_IGNORE ) {
            ; 
        } else

        if ( xml_param.info.cmd == CMD_ID_DELETE ) {
            ; 
        } else
                
        if ( xml_param.info.cmd == CMD_ID_NAME ) {
            if ( xml_param.p [0].empty () ) {
                RET_FALSE;
            }
            xml_param.info.t = CMD_ID_NAME;
            xml_param.info.p = xml_param.p [0];
        } else

        if ( xml_param.info.cmd == CMD_ID_AREA ) {
            if ( !MapAreaTypes (xml_param) ) {
                RET_FALSE;
            }
        } else

        if ( xml_param.info.cmd == CMD_ID_ROAD ) {
            if ( !MapWayTypes (xml_param) ) {
                RET_FALSE;
            }
        } else 
        
        {
            RET_FALSE;
        }

        g_lex_list_ways.push_back (xml_param.info);

        return true;

    }

    if ( target_name == "r" ) {

        if ( xml_param.info.cmd == CMD_ID_IGNORE ) {
            ;
        } else

        if ( xml_param.info.cmd == CMD_ID_DELETE ) {
            ;
        } else

        {
            RET_FALSE;
        }

        return true;
    }

    RET_FALSE;
}

bool LoadConfigFile (string cfg_file_name) {

    string str;
    ifstream file (cfg_file_name);

    if ( !file ) {
        RET_FALSE;
    }

    int line_id = 0;

    while ( getline (file, str) ) {

        line_id++;

        trim (str, " \t");

        if ( !ProcessCfgLine (str) ) {
            cout << "Bad CFG file at line: " << line_id << " (" << str << ") " << endl;
            RET_FALSE;
        }
    }

    return true;
}

void AddTag (string name) {

    OsmObject new_tag;

    g_xml_ctx.push_front (new_tag);
    g_xml_ctx.front ().node_name = name;
    g_xml_ctx.front ().node.lat = GEO_COORDINATE_ERROR;
    g_xml_ctx.front ().node.lon = GEO_COORDINATE_ERROR;
}

bool ProcessTag (string name, string val) {

    static const char * skip_list[] = {
        "timestamp",     "version",     "minlat",      "maxlat",
        "minlon",        "maxlon",      "generator",   "version",
        "changeset",     "timestamp",   "user",        "uid",
        "copyright",     "attribution", "license",     "visible"
    };

    for ( int i = 0; i < ARRAY_SIZE (skip_list); i++ ) {
        if ( name == skip_list [i] ) {
            return true;
        }
    }

    if ( g_xml_ctx [0].node_name == "node" ) {

        if ( name == "id" ) {
            g_xml_ctx.front ().id = stoll (val);
            return true;
        }

        if ( name == "lat" ) {
            g_xml_ctx.front ().node.lat = stod (val);
            return true;
        }

        if ( name == "lon" ) {
            g_xml_ctx.front ().node.lon = stod (val);
            return true;
        }
    }

    if ( g_xml_ctx [0].node_name == "tag" ) {

        if ( name == "k" ) {
            g_xml_ctx.front ().tmp_tag.k = val;
            return true;
        }

        if ( name == "v" ) {
            g_xml_ctx.front ().tmp_tag.v = val;
            return true;
        }
    }

    if ( g_xml_ctx [0].node_name == "way" ) {

        if ( name == "id" ) {
            g_xml_ctx.front ().id = stoll (val);
            return true;
        }

        if ( name == "ref" ) {
            uint64_t id = stoll (val);
            g_xml_ctx.front ().obj_ref = id;
            return true;
        }
    }

    if ( g_xml_ctx [1].node_name == "way" ) {
        if ( g_xml_ctx [0].node_name == "nd" ) {
            uint64_t idx = stoll (val);
            g_xml_ctx [0].refs.push_back (idx);
            return true;
        }
    }

    if ( g_xml_ctx [0].node_name == "relation" ) {

        if ( name == "id" ) {
            g_xml_ctx.front ().id = stoll (val);
            return true;
        }
    }

    if ( g_xml_ctx [1].node_name == "relation" ) {
        if ( g_xml_ctx [0].node_name == "member" ) {

            if ( name == "type" ) {
                if ( val == "node" ) {
                    g_xml_ctx.front ().tmp_member.type = MEMBER_TYPE_NODE;
                    return true;
                }
                if ( val == "way" ) {
                    g_xml_ctx.front ().tmp_member.type = MEMBER_TYPE_WAY;
                    return true;
                }
                if ( val == "relation" ) {
                    g_xml_ctx.front ().tmp_member.type = MEMBER_TYPE_REL;
                    return true;
                }
            } else

                if ( name == "ref" ) {
                    g_xml_ctx.front ().tmp_member.id = stoll (val);
                    return true;
                }

            if ( name == "role" ) {
                if ( val == "outer" ) {
                    g_xml_ctx.front ().tmp_member.role = MEMBER_ROLE_OUTER;
                    return true;
                }
                if ( val == "iner" ) {
                    g_xml_ctx.front ().tmp_member.role = MEMBER_ROLE_INNER;
                    return true;
                }
                if ( val == "subarea" ) {
                    g_xml_ctx.front ().tmp_member.role = MEMBER_ROLE_AREA;
                    return true;
                }

                return true;
            }
        }
    }

    cout << "Unknown tag: " << name << endl;

    RET_FALSE;
}

bool AddName (OsmObject& xml_info, string collection, string def_lang, string k, string v) {

    string str_lang;

    auto spliter = k.find (':');

    if ( spliter == string::npos ) {
        str_lang = def_lang;
    } else {
        str_lang = k.substr (spliter + 1, k.size ());
    }

    if ( str_lang.empty () ) {
        str_lang = def_lang;
    }

    bool is_found = false;
    auto name = xml_info.names.begin ();

    while ( name != xml_info.names.end () ) {
        if ( name->lang == str_lang ) {
            is_found = true;
            break;
        }
        name++;
    }

    if ( ! is_found ) {

        OSM_TEXT_INFO name;

        name.lang = str_lang;
        name.text = v;

        xml_info.names.push_back (name);
    }

    return true;
}

bool UpdateNodeInfo (OsmObject& xml_info) {

#if LOG_UNKNOWN_NODE
    bool tag_ignored = false;
#endif
    bool tag_deleted = false;

    xml_info.node.type = OBJ_ID_DEFAULT;

    auto it_param = xml_info.params.begin ();
    while ( it_param != xml_info.params.end () ) {
        
        tag_deleted = false;
        #if LOG_UNKNOWN_NODE
            tag_ignored = true;
        #endif

        if ( xml_info.node.type == OBJ_ID_DELETE ) {
            break;
        }

        for ( auto it_lex = g_lex_list_nodes.begin (); it_lex != g_lex_list_nodes.end (); it_lex++ ) {

            if ( LexCmp (it_param->k, it_lex->k) ) {
                if ( LexCmp (it_param->v, it_lex->v) ) {

                    #if LOG_UNKNOWN_NODE
                        tag_ignored = false;
                    #endif

                    if ( it_lex->cmd == CMD_ID_IGNORE ) {
                        it_param = xml_info.params.erase (it_param);
                        tag_deleted = true;
                        break;
                    } else
                    if ( it_lex->t & CMD_ID_DELETE ) {
                        xml_info.node.type = OBJ_ID_DELETE;
                        break;
                    } else 
                    if ( it_lex->t & CMD_ID_NAME ) {
                        AddName (xml_info, "n", it_lex->p, it_param->k, it_param->v);
                    } else
                    if ( it_lex->t & CMD_ID_POSITION ) {
                        xml_info.node.type |= it_lex->t;
                    }
                }
            }

        }

        #if LOG_UNKNOWN_NODE
            if ( tag_ignored ) {
                cout << "Tag: k=" << it_param->k << " v=" << it_param->v;
                cout << endl;
            }
        #endif

        if ( !tag_deleted ) {
            it_param++;
        }

    }

    return true;
}

bool StoreNodeInfo (OsmObject& xml_info) {

    OsmNameInfo name_info;
    OsmNodeInfo node_info;

    for ( auto it = xml_info.names.begin (); it != xml_info.names.end (); it++ ) {

        name_info.lang = it->lang;
        name_info.text = it->text;

        g_osm_node_names_map.emplace (xml_info.id, name_info);
    }

    node_info.type   = xml_info.node.type;
    node_info.lat    = xml_info.node.lat;
    node_info.lon    = xml_info.node.lon;
    node_info.in_use = false;

    if ( (xml_info.node.type & OBJ_ID_DELETE) == 0 ) {
        g_osm_nodes_map.emplace (xml_info.id, node_info);
    }

    return true;
}

bool UpdateWayInfo (OsmObject& xml_info) {

    for ( auto it_param = xml_info.params.begin (); it_param != xml_info.params.end (); it_param++ ) {

        // if ( LexCmp (it_param->k, "lanes") ) {
        //     xml_info.way.lanes = atoll (it_param->v.c_str ());
        //     continue;
        // }

        for ( auto it_lex = g_lex_list_ways.begin (); it_lex != g_lex_list_ways.end (); it_lex++ ) {
            if ( LexCmp (it_param->k, it_lex->k) ) {
                if ( LexCmp (it_param->v, it_lex->v) ) {

                    if ( it_lex->t == CMD_ID_NAME ) {
                        AddName (xml_info, "w", it_lex->p, it_param->k, it_param->v);
                    }

                    xml_info.way.type |= it_lex->t;
                    break;
                }
            }
        }

    }

    if ( xml_info.params.size () > 0 ) {
        if ( xml_info.way.type == 0 ) {
            cout << "Way skiiped: " << xml_info.id << endl;
        }
    }

    OsmNodeMap::iterator node_ptr;
    GEO_COORD coord;

    for ( auto it = xml_info.refs.begin (); it != xml_info.refs.end (); it++ ) {
        node_ptr = g_osm_nodes_map.find (*it);
        if ( node_ptr == g_osm_nodes_map.end () ) {
            return false;
        }
        node_ptr->second.in_use = true;

        coord.lat = node_ptr->second.lat;
        coord.lon = node_ptr->second.lon;

        xml_info.way.nodes.emplace_back (coord);
    }

    if ( xml_info.way.type != 0 ) {
        xml_info.way.in_use = true;
    }

    return true;
}

bool StoreWayInfo (OsmObject& xml_info) {

    OsmNameInfo name_info;
    uint64_t    msg_id;

    for ( auto it = xml_info.names.begin (); it != xml_info.names.end (); it++ ) {

        name_info.lang = it->lang;
        name_info.text = it->text;

        msg_id  = xml_info.id;
        msg_id |= 0x8000000000000000LLU;

        g_osm_node_names_map.emplace (msg_id, name_info);
    }

    if ( (xml_info.way.type & OBJ_ID_DELETE) == 0 ) {
        g_osm_ways_map.emplace (xml_info.id, xml_info.way);
    }

    return true;
}

bool UpdateRelInfo (OsmObject& xml_info, bool& is_ignored) {

    OSM_PARAM_TAG ignores[] = {
        {"waterway",            "river"},
        {"boundary",            "administrative"},
        {"route",               "bicycle"},
        {"type",                "network"},
        {"type",                "route"},
        {"type",                "boundary"},
        {"type",                "collection"},
        {"type",                "multipolygon"},
        {"type",                "site"},
        {"type",                "restriction"},
        {"type",                "public_transport"},
        {"type",                "enforcement"},
        {"public_transport",    "stop_area"},
        {"route",               "hiking"},
        {"route",               "power"},
        {"route",               "tracks"},
        {"route",               "bicycle"},          
        {"type",                "turnlanes:lengths"},
        {"type",                "turnlanes:turns"},
        {"type",                "junction"},
        {"type",                "level"},
        {"type",                "rf"},
        {"type",                "give_way"},
        {"type",                "stop"},
        {"type",                "associatedStreet"},
        {"type",                "pipeline"},
        {"type",                "traffic_mirror"},
        {"restriction",         "no_left_turn"}
    };

    OSM_PARAM_TAG accepted[] = {
        // <tag k="type" v="route_master"/>  <tag k="route_master" v="tram"/>  <tag k="ref" v="12"/>
        {"type",                "route_master"},      // 
        {"type",                "street"},            // 
        {"type",                "water"},             // 
        {"type",                "bridge"},            // 
        {"type",                "building"},          // 
        {"type",                "tunnel"},            // 
        {"building",            "yes"},               // 
        {"building",            "residential"},       // 
        {"building",            "industrial"},        // 
        {"landuse",             "forest"},            //
        {"landuse",             "meadow"},            //
        {"landuse",             "village_green"}      // 
    };

    is_ignored = false;

    for ( auto it = xml_info.params.begin (); it != xml_info.params.end (); it++ ) {

        for ( int i = 0; i < ARRAY_SIZE (ignores); i++ ) {
            if ( it->k == ignores [i].k ) {
                if ( it->v == ignores [i].v ) {
                    is_ignored = true;
                    return true;
                }
            }
        }
    }

    for ( auto it = xml_info.params.begin (); it != xml_info.params.end (); it++ ) {
        for ( int i = 0; i < ARRAY_SIZE (accepted); i++ ) {
            if ( it->k == accepted [i].k ) {
                if ( it->v == accepted [i].v ) {
                    is_ignored = true;
                    return true;
                }
            }
        }
    }

    cout << "Skipped Relation: " << xml_info.id << endl;
    return true;
}

bool StoreRelInfo (OsmObject& xml_info) {

    return true;
}

bool CloseTag (string name) {

    OsmObject xml_info;

    if ( g_xml_ctx.front ().node_name != name ) {
        cout << "Wrong tag closed: " << g_xml_ctx.front ().node_name << " -> " << name << endl;
        RET_FALSE;
    }

    xml_info = g_xml_ctx.front ();
    g_xml_ctx.pop_front ();

    if ( xml_info.node_name == "osm" ) {
        return true;
    }

    if ( xml_info.node_name == "bounds" ) {
        return true;
    }

    if ( xml_info.node_name == "tag" ) {
        g_xml_ctx.front ().params.push_back (xml_info.tmp_tag);
        return true;
    }

    if ( xml_info.node_name == "node" ) {
        UpdateNodeInfo (xml_info);
        StoreNodeInfo (xml_info);
        return true;
    }

    if ( xml_info.node_name == "nd" ) {
        g_xml_ctx.front ().refs.emplace_back (xml_info.obj_ref);
        return true;
    }

    if ( xml_info.node_name == "way" ) {
        UpdateWayInfo (xml_info);
        StoreWayInfo (xml_info);
        return true;
    }

    if ( xml_info.node_name == "member" ) {
        g_xml_ctx.front ().members.emplace_back (xml_info.tmp_member);
        return true;
    }

    if ( xml_info.node_name == "relation" ) {
        bool is_ignored;

        UpdateRelInfo (xml_info, is_ignored);

        if ( !is_ignored ) {
            StoreRelInfo (xml_info);
        }
        return true;
    }

    RET_FALSE;
}


int main (int argc, char* argv[]) {

    string      tag_name;
    int         offset = 0;
    int         stop   = 0;
    uint64_t    cnt = 0;

    if ( argc != 4 ) {
        cout << "Filtering [config_file_name] [in_file_name] [cache_file_name]" << endl;
        return -1;
    }

    if ( ! LoadConfigFile(argv [1]) ) {
        cout << "Cannot load config file name: " << argv [1] << endl;
        return -2;
    }

    g_osm_file_name = argv [2];

    Xml::Inspector<Xml::Encoding::Utf8Writer> inspector (argv [2]);
    Xml::Inspected id;

    while ( inspector.Inspect () ) {
    
        cnt++;

        if ( (cnt % 100000) == 0 ) {
            // cout << "\r";
            // cout << "Nodes cnt: " << g_osm_nodes_map.size ()        << "; ";
            // cout << "Ways cnt: "  << g_osm_ways_map.size ()         << "; ";
            // cout << "Rels cnt: "  << 0                              << "; ";
            // cout << "Names cnt: " << g_osm_node_names_map.size ()   << ".";
        }

        id = inspector.GetInspected ();

        switch ( id ) {

            case Xml::Inspected::StartTag:
                tag_name = inspector.GetName ();
                AddTag (tag_name);
                for ( int i = 0; i < inspector.GetAttributesCount (); i++ ) {
                    ProcessTag (inspector.GetAttributeAt (i).Name, inspector.GetAttributeAt (i).Value);
                }
                break;

            case Xml::Inspected::EndTag:
                tag_name = inspector.GetName ();
                CloseTag (tag_name);
                break;

            case Xml::Inspected::EmptyElementTag:
                tag_name = inspector.GetName ();
                AddTag (tag_name);
                for ( int i = 0; i < inspector.GetAttributesCount (); i++ ) {
                    ProcessTag (inspector.GetAttributeAt (i).Name, inspector.GetAttributeAt (i).Value);
                }
                CloseTag (tag_name);
                break;

            case Xml::Inspected::Text:
                break;

            case Xml::Inspected::Comment:
                break;

            default:
                break;
        }
    }

    return 0;
}
