// stop.h

class StopAudioClientOnExit {
public:
    StopAudioClientOnExit(IAudioClient *p) : m_p(p) {}
    ~StopAudioClientOnExit() {
        if (NULL != m_p) {
            m_p->Stop();
        }
    }

private:
    IAudioClient *m_p;
};
