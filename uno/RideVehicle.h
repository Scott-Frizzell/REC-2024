class RideVehicle {
	private:
		int location;
		byte id;
		int angle;
		unsigned long lastPing;
    int battery;
	public:
		RideVehicle(byte id) {
			this.location = -1;
			this.angle = 0;
			this.lastPing = millis();
			this.id = id;
      this.battery = 100;
		}
		RideVehicle() {
			this.location = -1;
			this.angle = 0;
			this.lastPing = 0;
			this.id = -1;
      this.battery = 0;
		}
		byte getId() { return this.id; }
		int getLocation() { return this.location; }
		int getAngle() { return this.angle; }
		unsigned long getLastPing() { return this.lastPing; }
    int getBattery() ( return this.battery; }
		void setLocation(int location) { this.location = location; }
		void setAngle(int angle) { this.angle = angle; }
		void setLastPing(unsigned long lastPing) { this.lastPing = lastPing; }
    void setBattery(int battery) { this.battery = battery; }
}
