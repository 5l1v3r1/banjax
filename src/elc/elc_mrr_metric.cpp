/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#include <elc_mrr_metric.hpp>
#include <dot11/frame.hpp>
#include <dot11/data_frame.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::elc_mrr_metric;

elc_mrr_metric::elc_mrr_metric(uint16_t rts_cts_threshold, uint16_t cw_time_us) :
   abstract_metric(),
   rts_cts_threshold_(rts_cts_threshold),
   n_pkt_succ_(0),
   t_pkt_succ_(0.0),
   t_pkt_fail_(0.0),
   packet_octets_(0),
   packet_count_(0),
   cw_time_us_(cw_time_us),
   mrr_(0.0)
{
}

elc_mrr_metric::elc_mrr_metric(const elc_mrr_metric& other) :
   abstract_metric(other),
   rts_cts_threshold_(other.rts_cts_threshold_),
   n_pkt_succ_(other.n_pkt_succ_),
   t_pkt_succ_(other.t_pkt_succ_),
   t_pkt_fail_(other.t_pkt_fail_),
   packet_octets_(other.packet_octets_),
   packet_count_(other.packet_count_),
   cw_time_us_(other.cw_time_us_),
   mrr_(other.mrr_)
{
}

elc_mrr_metric&
elc_mrr_metric::operator=(const elc_mrr_metric& other)
{
   if(&other != this) {
      abstract_metric::operator=(other);
      rts_cts_threshold_ = other.rts_cts_threshold_;
      n_pkt_succ_ = other.n_pkt_succ_;
      t_pkt_succ_ = other.t_pkt_succ_;
      t_pkt_fail_ = other.t_pkt_fail_;
      packet_octets_ = other.packet_octets_;
      packet_count_ = other.packet_count_;
      cw_time_us_ = other.cw_time_us_;
      mrr_ = other.mrr_;
   }
   return *this;
}

elc_mrr_metric::~elc_mrr_metric()
{
}

void
elc_mrr_metric::add(buffer_sptr b)
{
   frame f(b);
   buffer_info_sptr info(b->info());
   data_frame_sptr df(f.as_data_frame());
   if(info->has(TX_FLAGS) && df) {
      uint32_t tx_flags = info->tx_flags();
      if(tx_flags & TX_FLAGS_FAIL) {
         t_pkt_fail_ += packet_fail_time(b);
      } else {
         ++n_pkt_succ_;
         t_pkt_succ_ += packet_succ_time(b);
         const uint32_t CRC_SZ = 4;
         packet_octets_ += b->data_size() + CRC_SZ;
         ++packet_count_;      }
   }
}

elc_mrr_metric*
elc_mrr_metric::clone() const
{
   return new elc_mrr_metric(*this);
}

void
elc_mrr_metric::compute(uint32_t junk)
{
   const double AVG_PKT_SZ = packet_octets_ / static_cast<double>(packet_count_);
   mrr_ = (n_pkt_succ_ * AVG_PKT_SZ) / (t_pkt_succ_ + t_pkt_fail_);

   n_pkt_succ_ = 0;
   t_pkt_succ_ = 0;
   t_pkt_fail_ = 0;
   packet_octets_ = 0;
   packet_count_ = 0;
   mrr_ = 0;
}

void
elc_mrr_metric::write(ostream& os) const
{
   os << "ELC-MRR: " << mrr_;
}

double
elc_mrr_metric::packet_succ_time(buffer_sptr b) const
{
   double usecs = 0.0;
   buffer_info_sptr info(b->info());
   vector<uint32_t> rates(info->rates());
   encoding_sptr enc(info->channel_encoding());
   uint8_t retries = rates.size() - 1;
   for(uint8_t i = 0; i < retries; ++i) {
      usecs += (cw_time_us_ ? cw_time_us_ : avg_contention_time(enc, i)) + frame_fail_time(b, rates[i]);
   }
   return usecs + (cw_time_us_ ? cw_time_us_ : avg_contention_time(enc, retries)) + frame_succ_time(b, rates[retries]);
}

double
elc_mrr_metric::packet_fail_time(buffer_sptr b) const
{
   double usecs = 0.0;
   buffer_info_sptr info(b->info());
   vector<uint32_t> rates(info->rates());
   encoding_sptr enc(info->channel_encoding());
   uint8_t retries = rates.size() - 1;
   for(uint8_t i = 0; i < retries + 1; ++i) {
      usecs += (cw_time_us_ ? cw_time_us_ : avg_contention_time(enc, i)) + frame_fail_time(b, rates[i]);
   }
   return usecs;
}

double 
elc_mrr_metric::frame_succ_time(buffer_sptr b, uint32_t rate_Kbs) const
{
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const bool PREAMBLE =  info->has(CHANNEL_FLAGS) && (info->channel_flags() & CHANNEL_PREAMBLE_SHORT);
   const uint32_t T_RTS_CTS = (rts_cts_threshold_ <= FRAME_SZ) ? rts_cts_time(enc, FRAME_SZ, PREAMBLE) : 0;
   const uint32_t T_DATA = enc->txtime(FRAME_SZ, rate_Kbs, PREAMBLE);
   const uint32_t ACK_SZ = 14;
   const uint32_t ACK_RATE = enc->response_rate(rate_Kbs);
   const uint32_t T_ACK = enc->txtime(ACK_SZ, ACK_RATE, PREAMBLE);
   return T_RTS_CTS + T_DATA + enc->SIFS() + T_ACK + enc->DIFS();
}

double
elc_mrr_metric::frame_fail_time(buffer_sptr b, uint32_t rate_Kbs) const
{
   buffer_info_sptr info(b->info());
   encoding_sptr enc(info->channel_encoding());

   const uint32_t CRC_SZ = 4;
   const uint32_t FRAME_SZ = b->data_size() + CRC_SZ;
   const bool PREAMBLE =  info->has(CHANNEL_FLAGS) && (info->channel_flags() & CHANNEL_PREAMBLE_SHORT);
   const uint32_t T_RTS_CTS = (rts_cts_threshold_ <= FRAME_SZ) ? rts_cts_time(enc, FRAME_SZ, PREAMBLE) : 0;
   const uint32_t T_DATA = enc->txtime(FRAME_SZ, rate_Kbs, PREAMBLE);
   return T_RTS_CTS + T_DATA + enc->SIFS() + enc->ACKTimeout() + enc->DIFS();
}
