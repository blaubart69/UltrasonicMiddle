class SensorSettings {

    const String FILENAME = "/lastSettings.json";

public:

    unsigned int avg_values = 10;
    unsigned int threshold_cm = 2;

    bool load_from_file(String* err);
    bool save_to_file(String* err);
};