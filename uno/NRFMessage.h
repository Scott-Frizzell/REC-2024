class NRFMessage {
    public:
        char msg[32];
        int len;
        NRFMessage(char* string, int _len) : len(_len) {
            for (int i = 0; i < _len; i++) {
                this->msg[i] = string[i];
            }
        }
        NRFMessage() : msg(""), len(0) {}
};
