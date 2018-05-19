#ifndef BORDERS_H
#define BORDERS_H

#include "pt.h"
#include "err.h"

/// read geojson file with country borders from
/// http://ec.europa.eu/eurostat/web/gisco/geodata/reference-data/administrative-units-statistical-units/countries
/// jansson api: http://jansson.readthedocs.io/en/2.8/apiref.html
/// geojson: http://geojson.org/geojson-spec.html

/// get a border for country (selected by 2-letter code) into dMLine
dMLine get_border(const char *rg_file, const char *country);

#endif
