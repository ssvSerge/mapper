#include <iostream>
#include <string>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <sstream>
#include <iomanip>
#include <functional> 


#include "..\_tools\XmlInspector.hpp"


using namespace std;

#define CFG_TEXT_DEFAULT                        "Default"
#define CFG_TEXT_IGNORE                         "Ignore"
#define CFG_TEXT_DELETE                         "Delete"
#define CFG_TEXT_NAME                           "Name"
#define CFG_TEXT_TRANSPORT                      "Transport"
#define CFG_TEXT_BUILDING                       "Building"
#define CFG_TEXT_ROAD                           "Road"
#define CFG_TEXT_AREA                           "Area"
#define CFG_TEXT_WAY                            "Way"
#define CFG_TEXT_CIRCLE                         "Circle"
#define CFG_TEXT_OPTION                         "Option"
#define CFG_TEXT_RAILWAY                        "Railway"
#define CFG_TEXT_WATERWAY                       "Waterway"
#define CFG_TEXT_WAY_NODE                       "Node"

#define CFG_PARAM_JUNCTION                      "Junction"
#define CFG_PARAM_BUS_STOP                      "BusStop"
#define CFG_PARAM_TRAM_STOP                     "TramStop"
#define CFG_PARAM_METRO_STOP                    "MetroStop"
#define CFG_PARAM_METRO_ENTRANCE                "MetroEntrance"
#define CFG_PARAM_TRIAN_STOP                    "TrainStop"

#define CFG_PARAM_NODE_CIRCLE                   "TurnCircle"
#define CFG_PARAM_WAY_PRIMARY                   "Primary" 
#define CFG_PARAM_WAY_SECONDARY                 "Secondary"
#define CFG_PARAM_WAY_RESIDENTIAL               "Residential"
#define CFG_PARAM_WAY_SERVICE                   "Service"
#define CFG_PARAM_WAY_TERTIARY                  "Tertiary" 
#define CFG_PARAM_WAY_TRUNK                     "Trunk" 
#define CFG_PARAM_WAY_MOTORWAY                  "Motorway"
#define CFG_PARAM_WAY_TRACK                     "Track" 
#define CFG_PARAM_WAY_STREET                    "Street" 
#define CFG_PARAM_RAILWAY_TRAIN                 "Train"
#define CFG_PARAM_RAILWAY_TRAM                  "Tram"
#define CFG_PARAM_RAILWAY_METRO                 "Metro"
#define CFG_PARAM_AREA_GRASS                    "Grass"
#define CFG_PARAM_AREA_FOREST                   "Forest"
#define CFG_PARAM_AREA_ASPHALT                  "Asphalt"
#define CFG_PARAM_AREA_ROCK                     "Rock"


#define CFG_PARAM_BUILDING_OUT                  "Out"
#define CFG_PARAM_BUILDING_IN                   "In"


#define CACHE_FILE_TEXT_NODE                    "Node:"
#define CACHE_FILE_OBJ_OPTION                   "\t"
#define CACHE_FILE_OBJ_OPTION_PRI_TYPE          "P:"
#define CACHE_FILE_OBJ_OPTION_SEC_TYPE          "S:"
#define CACHE_FILE_OBJ_OPTION_NAME              "N:"
#define CACHE_FILE_OBJ_OPTION_LANG_DELIM        ":"
#define CACHE_FILE_OBJ_DELIM                    " "


#define GEO_COORDINATE_ERROR        (-1000)
#define GEO_ID_ERROR                   (-1)


#define ARRAY_SIZE(x)           ( sizeof(x) / sizeof(x[0]) )
#define SET_FALSE(var)          { var=false; cout << "Error ar: " << __LINE__ << endl; }
#define RET_FALSE               { cout << "Error at: " << __LINE__ << endl; return false; }


typedef enum tag_xml_prime_type_id {
    ID_TYPE_DEFAULT = 0,
    ID_NODE_IGNORE,
    ID_NODE_DELETE,
    ID_NODE_PUBLIC_TRANSPORT,
    ID_NODE_BUILDING,
    ID_NODE_NAME,
    ID_WAY_NODE,
    ID_WAY_ROAD,
    ID_WAY_RAILWAY,
    ID_WAY_WATERWAY,
    ID_BUILDING,
    ID_PATH_AREA,
    ID_PATH_WAY,
    ID_PATH_CIRCLE,
    ID_OPTION,
    ID_NODE_LAST_ID
}   XML_PRIME_TYPE;


typedef set<XML_PRIME_TYPE>         PRIME_TYPES_LIST;


typedef enum tag_xml_sub_type_id {
    XML_PARAM_NONE = 0,
    XML_PARAM_IGNORE,
    XML_PARAM_PT_STOP_BUS,
    XML_PARAM_PT_STOP_TRAM,
    XML_PARAM_PT_STOP_METRO,
    XML_PARAM_PT_STOP_TRIAN,
    XML_PARAM_PT_TURNCIRCLE,
    XML_PARAM_BUILDING_IN,
    XML_PARAM_BUILDING_OUT,
    XML_PARAM_PT_JUNCTION,
    XML_PARAM_WAY_PRIMARY,
    XML_PARAM_WAY_RESIDENTIAL,
    XML_PARAM_WAY_SECONDARY,
    XML_PARAM_WAY_TERTIARY,
    XML_PARAM_WAY_SERVICE,
    XML_PARAM_WAY_TRUNK,
    XML_PARAM_WAY_MOTORWAY,
    XML_PARAM_WAY_TRACK,
    XML_PARAM_WAY_STREET,
    XML_PARAM_RAILWAY_TRAIN,
    XML_PARAM_RAILWAY_TRAM,
    XML_PARAM_RAILWAY_METRO_STATION,
    XML_PARAM_RAILWAY_METRO_ENTRANCE,
    XML_PARAM_AREA_ASPHALT,
    XML_PARAM_AREA_ROCK,
    XML_PARAM_LAST_ID
}   XML_SUB_TYPE;


typedef set<XML_SUB_TYPE>           SUB_TYPES_LIST;


typedef struct tag_name_info {
    string     lang;
    string     name;
}   NAME_INFO;


typedef list<NAME_INFO>             NAMES_LIST;


typedef enum tag_child_type {
    CHILD_TYPE_UNKNOWN = 0,
    CHILD_TYPE_NODE    = 1,
    CHILD_TYPE_WAY     = 2,
    CHILD_TYPE_REL     = 3,
    CHILD_TYPE_LAST_ID
}   CHILD_TYPE;


typedef enum tag_role_type {
    ROLE_TYPE_UNKNOWN  = 0,
    ROLE_TYPE_INNER    = 1,
    ROLE_TYPE_OUTER    = 2,
    ROLE_TYPE_USER     = 3,
    ROLE_TYPE_LAST_ID
}   ROLE_TYPE;

    
class NODE_INFO {
    public:
        NODE_INFO() {
            obj_id   = -1;
            obj_type = CHILD_TYPE_UNKNOWN;
            obj_role = "";
        }

    public:
        uint64_t   obj_id;
        CHILD_TYPE obj_type;
        string     obj_role;

};


typedef vector<NODE_INFO> NODES_LIST;


typedef struct tag_osm_param {
    string   obj_k;
    string   obj_v;
}   OSM_PARAM;


typedef list<OSM_PARAM>   OSM_PARAMS_LIST;


class MapObject {

    public:
        MapObject() {
            Clear();
        }

    public:
        void Clear() {
            used            = false;
            deleted         = false;
            obj_id          = GEO_ID_ERROR;
            obj_lat         = GEO_COORDINATE_ERROR;
            obj_lon         = GEO_COORDINATE_ERROR;
            obj_ref         = GEO_ID_ERROR;
            obj_role        = ROLE_TYPE_UNKNOWN;
            obj_type        = CHILD_TYPE_UNKNOWN;
            node_name.clear();
            obj_k.clear();
            obj_v.clear();
            prime_types.clear();
            sub_types.clear();
            names.clear();
            nodes.clear();
            params.clear();
        }

    public:
        string              node_name;        // XML имя объекта (node, way, relation, nd, tag, member)

    public:
        uint64_t            obj_id;           // ID объекта id="12312"
        double              obj_lat;          // широта 
        double              obj_lon;          // долгота
        string              obj_k;            // параметр "k" объекта
        string              obj_v;            // параметр "v" объекта
        uint64_t            obj_ref;          // параметр ref для WAY и RELATION
        CHILD_TYPE          obj_type;         // тип объекта (node, way, relation) для объекта RELATION
        ROLE_TYPE           obj_role;         // функция объекта (outer, iner) для RELATION

    public:
        bool                used;             // TRUE в том случае, если объект используется в карте
        bool                deleted;          // TRUE если объект необходимо удалить (опции proposed, completted)
        PRIME_TYPES_LIST    prime_types;      // Список декодированных базовых типов
        SUB_TYPES_LIST      sub_types;        // Список подтипов
        NAMES_LIST          names;            // Список имен
        NODES_LIST          nodes;            // Список элементов в объекте WAY или RELATION
        OSM_PARAMS_LIST     params;           // Список тэгов k="" и v=""
};


typedef map<uint64_t, MapObject>  OBJ_MAP;


typedef OBJ_MAP::iterator       OBJ_MAP_POS;


typedef struct tag_xml_lex_info {
    string          k;
    string          v;
    XML_PRIME_TYPE  t;
    string          params [16];
}   XML_LEX_INFO;


typedef vector<XML_LEX_INFO>  LEX_LIST;


typedef struct tag_prime_lex {
    string              s;
    XML_PRIME_TYPE      t;
}   PRIME_LEX;


typedef struct tag_sub_lex {
    string              s;
    XML_SUB_TYPE        t;
}   SUB_LEX;


LEX_LIST            g_lex_list_nodes;
LEX_LIST            g_lex_list_ways;
LEX_LIST            g_lex_list_rels;

OBJ_MAP             g_nodes;
OBJ_MAP             g_ways;
OBJ_MAP             g_rels;
deque<MapObject>    g_xml_ctx;


static const PRIME_LEX prime_lex_list[] = {
    { CFG_TEXT_DEFAULT,             ID_TYPE_DEFAULT },
    { CFG_TEXT_IGNORE,              ID_NODE_IGNORE },
    { CFG_TEXT_DELETE,              ID_NODE_DELETE },
    { CFG_TEXT_NAME,                ID_NODE_NAME },
    { CFG_TEXT_TRANSPORT,           ID_NODE_PUBLIC_TRANSPORT },
    { CFG_TEXT_ROAD,                ID_WAY_ROAD },
    { CFG_TEXT_RAILWAY,             ID_WAY_RAILWAY },
    { CFG_TEXT_BUILDING,            ID_BUILDING },
    { CFG_TEXT_AREA,                ID_PATH_AREA },
    { CFG_TEXT_WAY,                 ID_PATH_WAY },
    { CFG_TEXT_CIRCLE,              ID_PATH_CIRCLE },
    { CFG_TEXT_WATERWAY,            ID_WAY_WATERWAY }, 
    { CFG_TEXT_OPTION,              ID_OPTION }
};


static const SUB_LEX sub_lex_list[] = {
    { CFG_PARAM_JUNCTION,           XML_PARAM_PT_JUNCTION },
    { CFG_PARAM_NODE_CIRCLE,        XML_PARAM_PT_TURNCIRCLE },
    { CFG_PARAM_BUS_STOP,           XML_PARAM_PT_STOP_BUS },
    { CFG_PARAM_TRAM_STOP,          XML_PARAM_PT_STOP_TRAM },
    { CFG_PARAM_METRO_STOP,         XML_PARAM_PT_STOP_METRO },
    { CFG_PARAM_TRIAN_STOP,         XML_PARAM_PT_STOP_TRIAN },
    { CFG_PARAM_BUILDING_IN,        XML_PARAM_BUILDING_IN },
    { CFG_PARAM_BUILDING_OUT,       XML_PARAM_BUILDING_OUT },
    { CFG_PARAM_WAY_RESIDENTIAL,    XML_PARAM_WAY_RESIDENTIAL },
    { CFG_PARAM_WAY_SECONDARY,      XML_PARAM_WAY_SECONDARY },
    { CFG_PARAM_WAY_SERVICE,        XML_PARAM_WAY_SERVICE },
    { CFG_PARAM_WAY_PRIMARY,        XML_PARAM_WAY_PRIMARY },
    { CFG_PARAM_WAY_TERTIARY,       XML_PARAM_WAY_TERTIARY },
    { CFG_PARAM_WAY_TRUNK,          XML_PARAM_WAY_TRUNK },
    { CFG_PARAM_WAY_MOTORWAY,       XML_PARAM_WAY_MOTORWAY },
    { CFG_PARAM_WAY_TRACK,          XML_PARAM_WAY_TRACK },
    { CFG_PARAM_WAY_STREET,         XML_PARAM_WAY_STREET },
    { CFG_PARAM_RAILWAY_TRAIN,      XML_PARAM_RAILWAY_TRAIN },
    { CFG_PARAM_RAILWAY_TRAM,       XML_PARAM_RAILWAY_TRAM },
    { CFG_PARAM_RAILWAY_METRO,      XML_PARAM_RAILWAY_METRO_STATION },
    { CFG_PARAM_METRO_ENTRANCE,     XML_PARAM_RAILWAY_METRO_ENTRANCE },
    { CFG_PARAM_AREA_ASPHALT,       XML_PARAM_AREA_ASPHALT },
    { CFG_PARAM_AREA_ROCK,          XML_PARAM_AREA_ROCK }
};


string& trim(string &s, string delims = " ") {

    size_t l, r, c;
    std::size_t found;

    for ( l = 0; l < s.length(); l++ ) {
        found = delims.find(s [l]);
        if ( found == string::npos ) {
            break;
        }
    }

    if ( l == s.length() ) {
        s = "";
        return s;
    }

    for ( r = s.length() - 1; r > l; r-- ) {
        found = delims.find(s [r]);
        if ( found == string::npos ) {
            break;
        }
    }

    c = (r - l + 1);
    s = s.substr(l, c);

    return s;
}


bool LexCmp(string xml_node, string cfg_param) {

    if ( xml_node.length() == 0 ) {
        RET_FALSE;
    }
    if ( cfg_param.length() == 0 ) {
        RET_FALSE;
    }

    if ( cfg_param == "*" ) {
        return true;
    }

    bool bIsTemplate = false;
    if ( cfg_param [cfg_param.length() - 1] == '*' ) {
        bIsTemplate = true;
        cfg_param.erase(cfg_param.length() - 1);
    }

    if ( cfg_param.length() > xml_node.length() ) {
        return false;
    }

    if ( cfg_param.length() == xml_node.length() ) {
        bool ret_val = cfg_param == xml_node;
        return ret_val;
    }

    if ( !bIsTemplate ) {
        return false;
    }

    string substr = xml_node.substr(0, cfg_param.length());

    if ( substr != cfg_param ) {
        return false;
    }

    return true;
}


bool MapPrimeCodeNode(string s, XML_PRIME_TYPE& t) {

    t = ID_NODE_LAST_ID;

    for ( size_t i = 0; i < ARRAY_SIZE(prime_lex_list); i++ ) {
        if ( prime_lex_list [i].s == s ) {
            t = prime_lex_list [i].t;
            break;
        }
    }

    if ( t == ID_NODE_LAST_ID ) {
        cout << "Unknown prine code: " << s << endl;
        RET_FALSE;
    }

    return true;
}


string PrimeTypeIdToName(XML_PRIME_TYPE prime_type) {

    size_t i;
    string ret_val;

    for ( i = 0; i < ARRAY_SIZE(prime_lex_list); i++ ) {
        if ( prime_type == prime_lex_list[i].t ) {
            ret_val = prime_lex_list[i].s;
            break;
        }
    }

    if ( i == ARRAY_SIZE(prime_lex_list) ) {
        ret_val = "PrimeTypeError";
    }

    return ret_val;
}


string SubTypeIdToName(XML_SUB_TYPE sub_type) {

    size_t i;
    string ret_val;

    for ( i = 0; i < ARRAY_SIZE(sub_lex_list); i++ ) {
        if ( sub_type == sub_lex_list[i].t ) {
            ret_val = sub_lex_list[i].s;
            break;
        }
    }

    if ( i == ARRAY_SIZE(sub_lex_list) ) {
        ret_val = "SubTypeError";
    }

    return ret_val;
}


bool MapSubCodeNode(string s, XML_SUB_TYPE& t) {

    t = XML_PARAM_LAST_ID;

    for ( size_t i = 0; i < ARRAY_SIZE(sub_lex_list); i++ ) {
        if ( sub_lex_list [i].s == s ) {
            t = sub_lex_list [i].t;
            break;
        }
    }

    if ( t == XML_PARAM_LAST_ID ) {
        cout << "Unknown sub code: " << s << endl;
        RET_FALSE;
    }

    return true;
}


bool AddName(MapObject& xml_info, string v, XML_LEX_INFO cfg_param) {

    string str_lang;

    auto spliter = v.find(':');

    if ( spliter == string::npos ) {
        str_lang = cfg_param.params [0];
    } else {
        str_lang = v.substr(spliter + 1, v.size());
    }

    if ( str_lang.empty() ) {
        str_lang = cfg_param.params [0];
    }

    bool is_found = false;
    auto name = xml_info.names.begin();

    while ( name != xml_info.names.end() ) {
        if ( name->lang == str_lang ) {
            is_found = true;
            break;
        }
        name++;
    }

    if ( !is_found ) {
        NAME_INFO new_name;
        new_name.lang = str_lang;
        new_name.name = xml_info.obj_v;
        xml_info.names.push_back(new_name);
    }

    return true;
}


bool ApplyConfigWayNode(MapObject& xml_info, XML_LEX_INFO cfg_param) {

    XML_SUB_TYPE sub_type;

    if ( cfg_param.t != ID_OPTION ) {
        xml_info.prime_types.insert(cfg_param.t);
    }

    for ( size_t i = 0; i < ARRAY_SIZE(cfg_param.params); i++ ) {
        if ( cfg_param.params [i].empty() ) {
            break;
        }
        if ( !MapSubCodeNode(cfg_param.params [i], sub_type) ) {
            RET_FALSE;
        }
        xml_info.sub_types.insert(sub_type);
    }

    return true;
}


bool ApplyConfigRel(XML_LEX_INFO cfg_param) {

    return true;
}


void LogCommand(const MapObject& xml_info, char code) {

    size_t len;
    size_t cnt;

    cout << code;
    cout << "\t";

    len = 40;
    if ( len <= xml_info.obj_k.length() ) {
        len = xml_info.obj_k.length() + 8;
    }

    len -= xml_info.obj_k.length();
    cnt = len / 8;
    if ( (len % 8) != 0 ) {
        cnt++;
    }

    cout << xml_info.obj_k;
    for ( size_t i = 0; i < cnt; i++ ) {
        cout << "\t";
    }

    cout << xml_info.obj_v;

    len = 64;
    if ( len <= xml_info.obj_v.length() ) {
        len = xml_info.obj_v.length() + 8;
    }

    len -= xml_info.obj_v.length();
    cnt = len / 8;
    if ( (len % 8) != 0 ) {
        cnt++;
    }

    for ( size_t i = 0; i < cnt; i++ ) {
        cout << "\t";
    }

    cout << "Ignore" << endl;
}


bool UpdateNodeInfoWithTag(MapObject& xml_info) {

    bool is_found = false;

    if ( xml_info.params.empty() ) {
        xml_info.prime_types.insert(ID_TYPE_DEFAULT);
        return true;
    }

    auto opt = xml_info.params.begin();
    bool is_shifted;

    while ( opt != xml_info.params.end() ) {

        is_found = false;

        xml_info.obj_k = opt->obj_k;
        xml_info.obj_v = opt->obj_v;

        is_shifted = false;
        for ( size_t i = 0; i < g_lex_list_nodes.size(); i++ ) {
            if ( LexCmp(xml_info.obj_k, g_lex_list_nodes [i].k) ) {
                if ( LexCmp(xml_info.obj_v, g_lex_list_nodes [i].v) ) {

                    is_found = true;

                    if ( g_lex_list_nodes [i].t == ID_NODE_DELETE ) {
                        // parameter type = "proposed" or "complete = no", etc.
                        xml_info.deleted = true;
                    } else
                    if ( g_lex_list_nodes [i].t == ID_NODE_IGNORE ) {
                        // parameter to be deleted like "source" or "web" etc.
                        is_shifted = true;
                        opt = xml_info.params.erase(opt);
                    } else
                    if ( g_lex_list_nodes [i].t == ID_NODE_NAME ) {
                        // parameter "name"
                        AddName(xml_info, xml_info.obj_v, g_lex_list_nodes [i]);
                    } else {
                        // all other types and subtypes.
                        ApplyConfigWayNode(xml_info, g_lex_list_nodes [i]);
                    }

                    break;
                }
            }
        }

        if ( !is_found ) {
            LogCommand(xml_info, 'n');
        }

        if ( !is_shifted ) {
            opt++;
        }
    }

    if ( xml_info.prime_types.empty() ) {
        xml_info.prime_types.insert(ID_TYPE_DEFAULT);
    }

    return true;
}


bool UpdateWayInfoWithTag(MapObject& xml_info) {

    if ( ! xml_info.params.empty() ) {

        auto opt = xml_info.params.begin();
        bool is_shifted = false;
        bool is_found = false;

        while ( opt != xml_info.params.end() ) {

            is_found = false;

            xml_info.obj_k = opt->obj_k;
            xml_info.obj_v = opt->obj_v;

            is_shifted = false;
            for ( size_t i = 0; i < g_lex_list_ways.size(); i++ ) {
                if ( LexCmp(xml_info.obj_k, g_lex_list_ways [i].k) ) {
                    if ( LexCmp(xml_info.obj_v, g_lex_list_ways [i].v) ) {

                        is_found = true;

                        if ( g_lex_list_ways [i].t == ID_NODE_DELETE ) {
                            // parameter type = "proposed" or "complete = no", etc.
                            xml_info.deleted = true;
                        } else
                        if ( g_lex_list_ways [i].t == ID_NODE_IGNORE ) {
                            // parameter to be deleted like "source" or "web" etc.
                            is_shifted = true;
                            opt = xml_info.params.erase(opt);
                        } else
                        if ( g_lex_list_ways [i].t == ID_NODE_NAME ) {
                            // parameter "name"
                            AddName(xml_info, xml_info.obj_v, g_lex_list_ways [i]);
                        } else {
                            // all other types and subtypes.
                            ApplyConfigWayNode(xml_info, g_lex_list_ways [i]);
                        }

                        break;
                    }
                }
            }

            if ( !is_found ) {
                LogCommand(xml_info, 'w');
            }

            if ( !is_shifted ) {
                opt++;
            }
        }

    }

    if ( !xml_info.deleted ) {
        bool bIsRoad = false;
        bool bIsAreaByParam = false;
        bool bIsAreaByNode = false;

        if ( xml_info.nodes.size() > 2 ) {
            uint64_t idFirst = xml_info.nodes [0].obj_id;
            uint64_t idLast = xml_info.nodes [xml_info.nodes.size() - 1].obj_id;
            if ( idFirst == idLast ) {
                bIsAreaByNode = true;
            }
        }

        for ( auto p_type = xml_info.prime_types.begin(); p_type != xml_info.prime_types.end(); p_type++ ) {
            if ( *p_type == ID_PATH_AREA ) {
                bIsAreaByParam = true;
            } else
                if ( *p_type == ID_WAY_ROAD ) {
                    bIsRoad = true;
                }
        }


        if ( bIsRoad ) {
            // Just skip it;
        } else
        if ( bIsAreaByParam ) {
            if ( !bIsAreaByNode ) {
                cout << "way id: " << xml_info.obj_id << " not CLOSED" << endl;
            }
        } else
        if ( !bIsAreaByParam ) {
            if ( bIsAreaByNode ) {
                cout << "way id: " << xml_info.obj_id << " not an AREA" << endl;
            }
        }
    }

    
    return true;
}


bool UpdateRelationInfoWithTag(const MapObject& xml_info) {

    bool is_found = false;

    if ( g_xml_ctx.size() < 2 ) {
        RET_FALSE;
    }

    for ( size_t i = 0; i < g_lex_list_rels.size(); i++ ) {
        if ( LexCmp(xml_info.obj_k, g_lex_list_rels [i].k) ) {
            if ( LexCmp(xml_info.obj_v, g_lex_list_rels [i].v) ) {
                ApplyConfigRel(g_lex_list_nodes [i]);
                is_found = true;
                break;
            }
        }
    }

    if ( !is_found ) {
        LogCommand(xml_info, 'r');
    }

    return true;
}


bool UpdateWayInfoWithNode(const MapObject& xml_info) {

    return true;
}


void UpdateRelationInfoWithMember(const MapObject& xml_info) {

}


void AddTag(string name) {

    MapObject new_tag;

    g_xml_ctx.push_front(new_tag);
    g_xml_ctx.front().node_name = name;
    g_xml_ctx.front().obj_lat = 0;
    g_xml_ctx.front().obj_lon = 0;
}


void CloseTag(string name) {

    MapObject xml_info;
    
    if ( g_xml_ctx.front().node_name != name ) {
        cout << "Wrong tag closed: " << g_xml_ctx.front().node_name << " -> " << name << endl;
        return;
    }

    xml_info = g_xml_ctx.front();
    g_xml_ctx.pop_front();

    if ( xml_info.node_name == "osm" ) {
        return;
    }

    if ( xml_info.node_name == "bounds" ) {
        return;
    }

    if ( xml_info.node_name == "node" ) {
        UpdateNodeInfoWithTag(xml_info);
        xml_info.params.clear();
        g_nodes.insert(pair<uint64_t, MapObject>(xml_info.obj_id, xml_info));
        return;
    }

    if ( xml_info.node_name == "way" ) {
        UpdateWayInfoWithTag(xml_info);
        xml_info.params.clear();
        g_ways.insert(pair<uint64_t, MapObject>(xml_info.obj_id, xml_info));
        return;
    }

    if ( xml_info.node_name == "relation" ) {
        g_rels.insert(pair<uint64_t, MapObject>(xml_info.obj_id, xml_info));
        return;
    }

    if ( xml_info.node_name == "tag" ) {
        OSM_PARAM new_param;
        new_param.obj_k = xml_info.obj_k;
        new_param.obj_v = xml_info.obj_v;
        g_xml_ctx.front().params.push_back(new_param);
        return;
    }

    if ( xml_info.node_name == "nd" ) {
        NODE_INFO new_node;
        if ( g_xml_ctx.front().node_name != "way" ) {
            cout << "Unknown tag sequence: " << g_xml_ctx.front().node_name << " -> " << xml_info.node_name << endl;
            return;
        }
        new_node.obj_id = xml_info.obj_ref;
        new_node.obj_type = CHILD_TYPE_NODE;
        g_xml_ctx.front().nodes.push_back(new_node);
        return;
    }

    if ( xml_info.node_name == "member" ) {
        NODE_INFO new_node;
        if ( g_xml_ctx.front().node_name != "relation" ) {
            cout << "Unknown tag sequence: " << g_xml_ctx.front().node_name << " -> " << xml_info.node_name << endl;
            return;
        }

        new_node.obj_type = xml_info.obj_type; // node way relation;
        new_node.obj_id   = xml_info.obj_ref;  // 
        new_node.obj_role = xml_info.obj_role; // 
        g_xml_ctx.front().nodes.push_back(new_node);
        return;
    }


    cout << "Unknown tag sequence: " << g_xml_ctx.front().node_name << " -> " << xml_info.node_name << endl;

}


bool ProcessTag(string name, string val) {

    if ( name == "k" ) {
        g_xml_ctx.front().obj_k = val;
    } else
    if ( name == "v" ) {
        g_xml_ctx.front().obj_v = val;
    } else
    if ( name == "id" ) {
        g_xml_ctx.front().obj_id = stoll(val);
    } else
    if ( name == "lat" ) {
        g_xml_ctx.front().obj_lat = stod(val);
    } else
    if ( name == "lon" ) {
        g_xml_ctx.front().obj_lon = stod(val);
    } else
    if ( name == "ref" ) {
        uint64_t ref = stoll(val);
        g_xml_ctx.front().obj_ref = stoll(val);
    } else
    if ( name == "type" ) {
        if ( val == "node" ) {
            g_xml_ctx.front().obj_type = CHILD_TYPE_NODE;
        } else
        if ( val == "way" ) {
            g_xml_ctx.front().obj_type = CHILD_TYPE_WAY;
        } else
        if ( val == "relation" ) {
            g_xml_ctx.front().obj_type = CHILD_TYPE_REL;
        } else {
            cout << "Unknown type: " << name << " " << val << endl;
            RET_FALSE;
        }

    } else
    if ( name == "ref" ) {

    } else 
    if ( name == "role" ) {

    } else {
        // cout << "Unknown parameter name: " << name << endl;
        // RET_FALSE;
    }

    return true;
}


bool ProcessCfgLine(string str) {

    if ( str.length() <= 1 ) {
        return true;
    }

    if ( str [0] == '#' ) {
        return true;
    }

    stringstream    ss(str);
    string          s_type;
    string          s_op;
    XML_LEX_INFO    xml_info;
    bool            is_failed = false;

    ss >> s_type;
    if ( s_type.empty() ) {
        RET_FALSE;
    }

    ss >> xml_info.k;
    ss >> xml_info.v;

    if ( xml_info.k.empty() || xml_info.v.empty() ) {
        RET_FALSE;
    }

    ss >> s_op;
    if ( s_op.empty() ) {
        RET_FALSE;
    }
    if ( !MapPrimeCodeNode(s_op, xml_info.t) ) {
        RET_FALSE;
    }

    for ( int i = 0; i < ARRAY_SIZE(xml_info.params); i++ ) {
        ss >> xml_info.params [i];
        if ( xml_info.params [i].empty() ) {
            break;
        }
    }

    if ( s_type == "n" ) {
        g_lex_list_nodes.push_back(xml_info);
    } else
    if ( s_type == "w" ) {
        g_lex_list_ways.push_back(xml_info);
    } else
    if ( s_type == "r" ) {
        g_lex_list_rels.push_back(xml_info);
    } else {
        RET_FALSE;
    }

    return true;
}


bool LoadConfigFile(string cfg_file_name) {

    string str;
    ifstream file(cfg_file_name);

    if ( !file ) {
        RET_FALSE;
    }

    int line_id = 0;

    while ( getline(file, str) ) {

        line_id++;

        trim(str, " \t");

        if ( !ProcessCfgLine(str) ) {
            cout << "Bad CFG file at line: " << line_id << " ("<< str << ") "<< endl;
            RET_FALSE;
        }
    }

    return true;
}


bool LoadCacheData(string file_name) {

    ifstream        in_file;
    string          data;
    MapObject       obj;
    XML_PRIME_TYPE  prime_type;
    XML_SUB_TYPE    sub_type;

    in_file.open(file_name);
    if ( !in_file ) {
        RET_FALSE;
    }

    while ( getline(in_file, data) ) {
        
        if ( data.empty() ) {
            continue;
        }

        if ( strncmp(data.c_str(), CACHE_FILE_TEXT_NODE, strlen(CACHE_FILE_TEXT_NODE) ) == 0 ) {

            if ( obj.obj_id != GEO_ID_ERROR ) {
                g_nodes.insert(pair<uint64_t, MapObject>(obj.obj_id, obj));
                obj.Clear();
            }

            istringstream   iss(data);
            string          name;
            uint64_t        id  = GEO_ID_ERROR;
            double          lat = GEO_COORDINATE_ERROR;
            double          lon = GEO_COORDINATE_ERROR;

            iss >> name;
            iss >> id;

            if ( id == GEO_ID_ERROR ) {
                cout << "Cache file error: POS: 1 (ID expected)" << endl;
                RET_FALSE;
            }

            iss >> lat;
            if ( lat == GEO_COORDINATE_ERROR ) {
                cout << "Cache file error: POS: 2 (LAT expected)" << endl;
                RET_FALSE;
            }

            iss >> lon;
            if ( lon == GEO_COORDINATE_ERROR ) {
                cout << "Cache file error: POS: 2 (LON expected)" << endl;
                RET_FALSE;
            }

            obj.obj_id  = id;
            obj.obj_lat = lat;
            obj.obj_lon = lon;

        }  else

        if ( strncmp(data.c_str(), CACHE_FILE_OBJ_OPTION, strlen(CACHE_FILE_OBJ_OPTION)) == 0 ) {
            
            stringstream   iss;
            string         val;

            iss << data;
            iss >> val;

            if ( val == CACHE_FILE_OBJ_OPTION_PRI_TYPE ) {
                iss >> val;
                if ( !MapPrimeCodeNode(val, prime_type) ) {
                    cout << "Cache file error: Unknown Prime Type" << endl;
                    RET_FALSE;
                }
                obj.prime_types.insert(prime_type);
            } else
            if ( val == CACHE_FILE_OBJ_OPTION_SEC_TYPE ) {
                iss >> val;
                if ( !MapSubCodeNode(val, sub_type) ) {
                    cout << "Cache file error: Unknown Sub Type" << endl;
                    RET_FALSE;
                }
                obj.sub_types.insert(sub_type);
            } else
            if ( val == CACHE_FILE_OBJ_OPTION_NAME ) {

                size_t      div_pos;
                NAME_INFO   name_info;

                val = iss.str().substr(iss.tellg());

                val = trim(val);
                div_pos = val.find(':');
                if ( div_pos == string::npos ) {
                    cout << "Cache file error: Unknown Language ID" << endl;
                    RET_FALSE;
                }
                name_info.lang = val.substr(0, div_pos);
                name_info.name = val.substr(div_pos+1, val.length());
                obj.names.push_back(name_info);

            } else {
                cout << "Cache file error: Unknown option type" << endl;
                RET_FALSE;
            }
        }

    }

    if ( obj.obj_id != GEO_ID_ERROR ) {
        g_nodes.insert(pair<uint64_t, MapObject>(obj.obj_id, obj));
    }

    return true;
}


void OutCacheFile (ofstream& out_file) {

    out_file.precision(7);


    OBJ_MAP_POS iter_nodes;
    for ( iter_nodes = g_nodes.begin(); iter_nodes != g_nodes.end(); iter_nodes++ ) {

        out_file << CACHE_FILE_TEXT_NODE << CACHE_FILE_OBJ_DELIM;
        out_file << setfill(' ') << setw(12) << iter_nodes->first << CACHE_FILE_OBJ_DELIM;
        out_file << fixed << iter_nodes->second.obj_lat << CACHE_FILE_OBJ_DELIM;
        out_file << fixed << iter_nodes->second.obj_lon;

        if ( !iter_nodes->second.prime_types.empty() ) {

            out_file << endl;
            out_file << CACHE_FILE_OBJ_OPTION;
            out_file << CACHE_FILE_OBJ_OPTION_PRI_TYPE;
            out_file << CACHE_FILE_OBJ_DELIM;
            for ( auto i = iter_nodes->second.prime_types.begin(); i != iter_nodes->second.prime_types.end(); i++ ) {
                out_file << PrimeTypeIdToName(*i) << CACHE_FILE_OBJ_DELIM;
            }

            if ( !iter_nodes->second.sub_types.empty() ) {
                out_file << endl;
                out_file << CACHE_FILE_OBJ_OPTION;
                out_file << CACHE_FILE_OBJ_OPTION_SEC_TYPE;
                out_file << CACHE_FILE_OBJ_DELIM;
                for ( auto i = iter_nodes->second.sub_types.begin(); i != iter_nodes->second.sub_types.end(); i++ ) {
                    out_file << SubTypeIdToName(*i) << CACHE_FILE_OBJ_DELIM;
                }
            }

            if ( !iter_nodes->second.names.empty() ) {
                for ( auto i = iter_nodes->second.names.begin(); i != iter_nodes->second.names.end(); i++ ) {
                    out_file << endl;
                    out_file << CACHE_FILE_OBJ_OPTION;
                    out_file << CACHE_FILE_OBJ_OPTION_NAME;
                    out_file << CACHE_FILE_OBJ_DELIM;
                    out_file << i->lang;
                    out_file << CACHE_FILE_OBJ_OPTION_LANG_DELIM;
                    out_file << i->name;
                }
            }
        }

        out_file << endl;
    }
}


int main (int argc, char* argv[]) {

    string tag_name;
    int offset = 0;
    int stop   = 0;
    ofstream out_file;

    if ( argc != 4 ) {
        cout << "Filtering [config_file_name] [in_file_name] [cache_file_name]" << endl;
        return -1;
    }

    if ( !LoadConfigFile(argv [1]) ) {
        cout << "Cannot load config file name: " << argv [1] << endl;
        return -2;
    }

    // LoadCacheData(argv [3]);

    out_file.open(argv [3]);
    if ( !out_file ) {
        cout << "Cannot create/open cache file: " << argv [3] << endl;
        return -3;
    }

    Xml::Inspector<Xml::Encoding::Utf8Writer> inspector(argv [2]);
    Xml::Inspected  id;

    while ( inspector.Inspect() ) {

        offset++;

        id = inspector.GetInspected();

        switch ( id ) {

            case Xml::Inspected::StartTag:
                tag_name = inspector.GetName();
                AddTag(tag_name);
                for ( int i = 0; i < inspector.GetAttributesCount(); i++ ) {
                    ProcessTag(inspector.GetAttributeAt(i).Name, inspector.GetAttributeAt(i).Value);
                }
                break;

            case Xml::Inspected::EndTag:
                tag_name = inspector.GetName();
                CloseTag(tag_name);
                break;

            case Xml::Inspected::EmptyElementTag:
                tag_name = inspector.GetName();
                AddTag(tag_name);
                for ( int i = 0; i < inspector.GetAttributesCount(); i++ ) {
                    ProcessTag(inspector.GetAttributeAt(i).Name, inspector.GetAttributeAt(i).Value);
                }
                CloseTag(tag_name);
                break;

            case Xml::Inspected::Text:
                std::cout << "Text value: " << inspector.GetValue() << "\n";
                break;

            case Xml::Inspected::Comment:
                std::cout << "Comment value: " << inspector.GetValue() << "\n";
                break;

            default:
                break;
        }
    }

    OutCacheFile(out_file);

    return 0;
}
