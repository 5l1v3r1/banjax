/* -*- mode C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 * 
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <airtime_metric_linux.hpp>

#include <dot11/frame.hpp>
#include <util/exceptions.hpp>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>
#include <sstream>

using namespace dot11;
using namespace net;
using namespace std;
using metrics::airtime_metric_linux;

airtime_metric_linux::airtime_metric_linux(encoding_sptr enc) :
   abstract_metric(),
   enc_(enc),
   last_rate_Kbs_(0),
   fail_avg_(0),
   airtime_(0.0)
{
}

airtime_metric_linux::airtime_metric_linux(const airtime_metric_linux& other) :
   abstract_metric(other),
   enc_(other.enc_),
   last_rate_Kbs_(other.last_rate_Kbs_),
   fail_avg_(other.fail_avg_),
   airtime_(other.airtime_)
{
}

airtime_metric_linux&
airtime_metric_linux::operator=(const airtime_metric_linux& other)
{
   if(this != &other) {
      abstract_metric::operator=(other);
      enc_ = other.enc_;
      last_rate_Kbs_ = other.last_rate_Kbs_;
      fail_avg_ = other.fail_avg_;
      airtime_ = other.airtime_;
   }
   return *this;
}

airtime_metric_linux::~airtime_metric_linux()
{
}

void
airtime_metric_linux::add(buffer_sptr b)
{
   frame f(b);
   frame_control fc(f.fc());
   const uint32_t CRC_SZ = 4;
   buffer_info_sptr info(b->info());
   if(DATA_FRAME == fc.type() && info->has(TX_FLAGS)) {
      bool tx_success = (0 == (info->tx_flags() & TX_FLAGS_FAIL));
      if(tx_success) {
         last_rate_Kbs_ = info->rate_Kbs();
      }
      fail_avg_ = ((80 * fail_avg_ + 5) / 100) + ((!tx_success) ? 20 : 0);
   }
}

airtime_metric_linux*
airtime_metric_linux::clone() const
{
   return new airtime_metric_linux(*this);
}

double
airtime_metric_linux::compute(uint32_t ignored_delta_us)
{
   const uint8_t ARITH_SHIFT = 8;
   const uint32_t UDP_SZ = 62;
   const uint32_t CRC_SZ = 4;
   const uint32_t TEST_FRAME_SZ = 8 * (1024 + UDP_SZ + CRC_SZ);
   const uint32_t S_UNIT = 1 << ARITH_SHIFT;
	const int32_t DEVICE_CONSTANT = 1 << ARITH_SHIFT;

   if(last_rate_Kbs_) {
      uint32_t err = (fail_avg_ << ARITH_SHIFT) / 100;
      uint32_t rate = last_rate_Kbs_ / 10;
      uint32_t tx_time = (DEVICE_CONSTANT + 10 * TEST_FRAME_SZ / rate);
      uint32_t estimated_retx = ((1 << (2 * ARITH_SHIFT)) / (S_UNIT - err));
      airtime_ = /* TEST_FRAME_SZ / static_cast<double>((tx_time * estimated_retx) >> (2 * ARITH_SHIFT)); */ tx_time;
   } else {
      airtime_ = 0;
   }
   return airtime_;
}

void
airtime_metric_linux::reset()
{
   // we can safely preserve fail_avg_ and last_rate_Kbs_
}

void
airtime_metric_linux::write(ostream& os) const
{
   os << "fail-avg: " << fail_avg_ << ", ";
   os << "last-rate: " << last_rate_Kbs_ << ", ";

   os << "Airtime-Linux: " << airtime_;
}
