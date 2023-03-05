#include "randomqueue.h"

RandomQueue::RandomQueue(linkspeed_bps bitrate, mem_b maxsize, QueueLogger *logger, mem_b drop)
    : Queue(bitrate, maxsize, logger), _drop(drop), _buffer_drops(0)
{
    _drop_th = _maxsize - _drop;
    _plr = 0.0;
}

void
RandomQueue::set_packet_loss_rate(double l)
{
    _plr = l;
}

    void
RandomQueue::receivePacket(Packet &pkt) 
{
    double drop_prob = 0;
    mem_b crt = _queuesize + pkt.size();

    if (_plr > 0.0 && drand() < _plr) {
        pkt.free();
        return;
    }

    if (crt > _drop_th)
        drop_prob = 1100.0 / _drop_th;

    if (crt > _maxsize || drand() < drop_prob) {
        if (_logger) _logger->logQueue(*this, QueueLogger::PKT_DROP, pkt);
        pkt.flow().logTraffic(pkt,*this,TrafficLogger::PKT_DROP);

        if (crt > _maxsize) {
            _buffer_drops ++;
        }
        pkt.free();
        return;
    }

    pkt.flow().logTraffic(pkt,*this,TrafficLogger::PKT_ARRIVE);
    bool queueWasEmpty = _enqueued.empty();
    _enqueued.push_front(&pkt);
    _queuesize += pkt.size();

    if (_logger) _logger->logQueue(*this, QueueLogger::PKT_ENQUEUE, pkt);

    if (queueWasEmpty) {
        assert(_enqueued.size()==1);
        beginService();
    }
}
