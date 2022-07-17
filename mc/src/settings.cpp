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
        this->set_avg_values  ( json_doc["avg_values"].as<int>());
        this->set_threshold_cm( json_doc["threshold_cm"].as<int>() );
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
    json_doc["threshold_cm"] = this->get_threshold_cm();

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
    json_doc["threshold_cm"] = this->threshold_mm * 10;

    serializeJson(json_doc, *json);

    return true;
}

int SensorSettings::get_avg_values() const { 
    return _avg_values; 
}

void SensorSettings::set_avg_values(const int val) {
    if ( val <= 0 ) {
        _avg_values = 1;
    }
    else {
        _avg_values = val;
    }
}

int  SensorSettings::get_threshold_cm() const {
    return this->threshold_mm * 10;
}

int  SensorSettings::get_threshold_mm() const {
    return this->threshold_mm;
}

void SensorSettings::set_threshold_cm(int newThresholdCm) {
    this->threshold_mm = newThresholdCm * 10;
}

