@echo off

SET NODES_START=0x00000000CE
SET NODES_END=0x0145676638
SET WAYS_START=0x0145676639
SET WAYS_END=0x0215697244
SET RELS_START=0x0215697245
SET RELS_END=0x021C542920


rem ..\_bin\Release\LargeFileCut.exe prague.osm       prague_nodes.osm  %NODES_END%   %RELS_END%
rem ..\_bin\Release\LargeFileCut.exe prague.osm       prague_rels.osm   %NODES_START% %WAYS_END%
rem ..\_bin\Release\LargeFileCut.exe prague.osm       prague_tmp.osm    %RELS_START%  %RELS_END%
rem ..\_bin\Release\LargeFileCut.exe prague_tmp.osm   prague_ways.osm   %NODES_START% %NODES_END%
rem del prague_tmp.osm

rem ..\_bin\Release\OsmiumTool.exe extract -b 14.3010,50.0287,14.6006,50.1351 -v --progress -F osm -f osm E:\Osm\czech_republic_2018_11_25_filtered.osm -o prague.osm
rem ..\_bin\Release\OsmiumTool.exe extract -b 14.3311,50.0329,14.3482,50.0431 -v --progress -F osm -f osm E:\Osm\czech_republic_2018_11_25_filtered.osm -o ohrada.osm


rem ..\_bin\Release\osm_idx.exe C:\Temp\osm_tools\_maps\tags.cfg C:\Temp\osm_tools\_maps\prague.osm cache.txt

rem ..\_bin\Release\osmium-tool.exe extract -b 14.3390,50.0448,14.3755,50.0585 -v --progress -F osm -f osm E:\Osm\czech_republic-2018-04-02.osm -o ohrada_full.osm
rem ..\_bin\Release\osm-filter.exe InFile=C:\Temp\osm_tools\_maps\ohrada_full.osm OutFile=C:\Temp\osm_tools\_maps\ohrada.osm Skip=version,timestamp,uid,user,changeset
rem ..\_bin\Release\osm_idx.exe C:\Temp\osm_tools\_maps\tags.cfg C:\Temp\osm_tools\_maps\prague.osm C:\Temp\osm_tools\_maps\cache.txt

rem ..\_bin\Release\OsmFilter.exe InFile=E:\Osm\czech_republic_2018_11_25.osm OutFile=E:\Osm\czech_republic_2018_11_25_filtered.osm Skip=source,version,timestamp,uid,user,changeset
rem ..\_bin\Release\OsmFilter.exe InFile=E:\Osm\czech_republic_2018_11_25_filtered.osm OutFile=C:\Projects\MAP\MapRender_02\_maps\prague.osm Skip=version,timestamp,uid,user,changeset
rem ..\_bin\Release\OsmIdx.exe tags.cfg prague.osm cache.txt

    ..\_bin\Release\OsmIdx.exe tags.cfg prague.osm cache.txt
