class SensorSettings {

    const String FILENAME = "/lastSettings.json";

public:
    
    bool load_from_file(String* err);
    bool save_to_file(String* err);
    bool current(String* json);

    int get_avg_values() const;
    void set_avg_values(const int val);

    int  get_threshold_cm() const;
    int  get_threshold_mm() const;
    void set_threshold_cm(int newThresholdCm);

private:

    unsigned int _avg_values = 10;
    unsigned int threshold_mm = 20;

};