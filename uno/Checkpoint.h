class Checkpoint {
    private:
        int id;
        int flag;
    public:
        Checkpoint() {
            id = millis(); // TODO: bad
            flag = 0;
        }

        void configure(int _id) {
            id = _id;
        }

        void set(int _id) {
            flag = _id;
        }

        int getId() {
            return id;
        }

        int test() {
            return flag;
        }

        int clear() {
            int temp = flag;
            flag = 0;
            return temp;
        }
};