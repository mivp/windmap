#!/bin/bash

GFS_DATE="20171105"
GFS_TIME="00"; # 00, 06, 12, 18
RES="0p25" # 0p25, 0p50 or 1p00
BBOX="leftlon=141&rightlon=147&toplat=-35&bottomlat=-40"
LEVEL="lev_10_m_above_ground=on"
GFS_URL="http://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_${RES}.pl?file=gfs.t${GFS_TIME}z.pgrb2.${RES}.f000&${LEVEL}&${BBOX}&dir=%2Fgfs.${GFS_DATE}${GFS_TIME}"

curl "${GFS_URL}&var_UGRD=on" -o utmp.grib
curl "${GFS_URL}&var_VGRD=on" -o vtmp.grib

#grib_set -r -s packingType=grid_simple utmp.grib utmp.grib
#grib_set -r -s packingType=grid_simple vtmp.grib vtmp.grib

#printf "{\"u\":`grib_dump -j utmp.grib`,\"v\":`grib_dump -j vtmp.grib`}" > tmp.json

#rm utmp.grib vtmp.grib
#DIR=`dirname $0`
#node ${DIR}/prepare.js ${1}/${GFS_DATE}${GFS_TIME}
#rm tmp.json
