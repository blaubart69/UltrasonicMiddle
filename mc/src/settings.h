class SensorSettings {

    const String FILENAME = "/lastSettings.json";

public:
    
    unsigned int threshold_cm = 2;

    bool load_from_file(String* err);
    bool save_to_file(String* err);
    bool current(String* json);

    int get_avg_values() const { return _avg_values; }
    void set_avg_values(const int val) {
        if ( val <= 0 ) {
            _avg_values = 1;
        }
        else {
            _avg_values = val;
        }
    }

private:

    unsigned int _avg_values = 10;

};