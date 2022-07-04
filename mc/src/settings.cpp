#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "settings.h"

bool SensorSettings::load_from_file(String* err)
{
    err->clear();

    StaticJsonDocument<512> json_doc;
    DeserializationError json_err;

    fs::File fd = LittleFS.open(FILENAME);
    if ( !fd ) {
        *err = "could not open ";
        *err += FILENAME;
    }
    else if ( (json_err = deserializeJson(json_doc,fd)) ){
        *err = "could not parse " + FILENAME + ". ";
        *err += json_err.c_str();
    }
    else {
        this->set_avg_values(json_doc["avg_values"].as<int>());
        this->threshold_cm = json_doc["threshold_cm"];
    }

    if ( fd ) { fd.close(); }

    return err->isEmpty();
}

bool SensorSettings::save_to_file(String* err)
{
    err->clear();
    StaticJsonDocument<512> json_doc;
    size_t bytesWritten;

    json_doc["avg_values"]   = this->get_avg_values();
    json_doc["threshold_cm"] = this->threshold_cm;

    fs::File fd = LittleFS.open(FILENAME, "w");
    if ( !fd ) {
        *err = "could not open ";
        *err += FILENAME;
    }
    else if ( (bytesWritten = serializeJson(json_doc,fd)) == 0 ) {
        *err = "0 bytes written to file";
    }
    else {
        err->clear();
    }

    if ( fd ) { fd.close(); }

    return err->isEmpty();
}

bool SensorSettings::current(String* json)
{
    StaticJsonDocument<512> json_doc;
    size_t bytesWritten;

    json_doc["avg_values"]   = this->get_avg_values();
    json_doc["threshold_cm"] = this->threshold_cm;

    serializeJson(json_doc, *json);

    return true;
}