/* -*- Mode: C++; tab-width: 3; -*- */

/* 
 * Copyright NICTA, 2011
 */

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <airtime_metric.hpp>
#include <airtime_metric_linux.hpp>
#include <elc_metric.hpp>
#include <elc_mrr_metric.hpp>
#include <etx_metric.hpp>
#include <fdr_metric.hpp>
#include <goodput_metric.hpp>
#include <iperf_metric_wrapper.hpp>
#include <legacy_elc_metric.hpp>
#include <metric.hpp>
#include <metric_damper.hpp>
#include <metric_decimator.hpp>
#include <metric_demux.hpp>
#include <metric_group.hpp>
#include <pdr_metric.hpp>
#include <pktsz_metric.hpp>
#include <residual.hpp>
#include <txc_metric.hpp>

#include <net/buffer_info.hpp>
#include <net/ofdm_encoding.hpp>
#include <net/wnic.hpp>
#include <net/wnic_encoding_fix.hpp>
#include <net/wnic_wallclock_fix.hpp>

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace boost;
using namespace boost::program_options;
using namespace net;
using namespace metrics;
using namespace std;

int
main(int ac, char **av)
{
   try {

      bool help;
      string enc_str, what;
      uint16_t beacon_time;
      uint16_t cw; 
      uint16_t damp;
      uint16_t mpdu_sz;
      uint16_t port_no;
      uint16_t rts_cts_threshold;
      uint32_t rate_Mbs;
      size_t window_sz;

      options_description options("program options");
      options.add_options()
         ("beacon,b", value(&beacon_time)->default_value(0), "time (in microseconds) lost sending own beacon")
         ("cw,c", value(&cw)->default_value(0), "size of contention window in microseconds (0 = compute average)")
         ("damping,d", value(&damp)->default_value(5), "size of damping window in seconds")
         ("encoding,e", value<string>(&enc_str)->default_value("OFDM"), "channel encoding")
         ("help,?", value(&help)->default_value(false)->zero_tokens(), "produce this help message")
         ("input,i", value<string>(&what)->default_value("mon0"), "input file/device name")
         ("link-rate,l", value<uint32_t>(&rate_Mbs)->default_value(54), "Maximum link rate in Mb/s")
         ("mpdu,m", value<uint16_t>(&mpdu_sz)->default_value(1536), "MPDU size used metric calculation")
         ("port,p", value<uint16_t>(&port_no)->default_value(50000), "port number used for ETX probes")
         ("rts-threshold,r", value<uint16_t>(&rts_cts_threshold)->default_value(UINT16_MAX), "RTS threshold level")
         ("window,w", value<size_t>(&window_sz)->default_value(10), "ETX probe windows")
         ;

      variables_map vars;       
      store(parse_command_line(ac, av, options), vars);
      notify(vars);   

      if(help) {
         cout << options << endl;
         exit(EXIT_SUCCESS);
      }

      encoding_sptr enc(encoding::get(enc_str));
   	metric_group_sptr proto(new metric_group);
      proto->push_back(metric_sptr(new  goodput_metric));
      proto->push_back(metric_sptr(new elc_metric(cw, rts_cts_threshold, beacon_time)));
//      proto->push_back(metric_sptr(new metric_decimator("ELC-1PC", metric_sptr(new elc_metric(cw, rts_cts_threshold)), 100)));
//      proto->push_back(metric_sptr(new metric_decimator("ELC-10PC", metric_sptr(new elc_metric(cw, rts_cts_threshold)), 10)));
      proto->push_back(metric_sptr(new metric_damper("Damped-ELC", metric_sptr(new elc_metric(cw, rts_cts_threshold, beacon_time)), damp)));
//      proto->push_back(metric_sptr(new elc_mrr_metric(cw, rts_cts_threshold)));
      proto->push_back(metric_sptr(new legacy_elc_metric(enc, rate_Mbs * 1000, mpdu_sz, rts_cts_threshold)));
      proto->push_back(metric_sptr(new airtime_metric(enc, rts_cts_threshold)));
//      proto->push_back(metric_sptr(new airtime_metric_linux(enc)));
//      proto->push_back(metric_sptr(new etx_metric(port_no, window_sz)));
      proto->push_back(metric_sptr(new fdr_metric));
      proto->push_back(metric_sptr(new txc_metric));
//      proto->push_back(metric_sptr(new residual(metric_sptr(new goodput_metric), "Residual")));
//      proto->push_back(metric_sptr(new residual(metric_sptr(new legacy_elc_metric(enc, rate_Mbs * 1000, mpdu_sz, rts_cts_threshold)), "RELC")));
      metric_sptr m(new iperf_metric_wrapper(metric_sptr(new metric_demux(proto))));

      wnic_sptr w(wnic::open(what));
      w = wnic_sptr(new wnic_wallclock_fix(w));
      if("OFDM" == enc_str) {
         w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_OFDM | CHANNEL_PREAMBLE_LONG));
      } else if("DSSS" == enc_str) {
         w = wnic_sptr(new wnic_encoding_fix(w, CHANNEL_CODING_DSSS | CHANNEL_PREAMBLE_LONG));
      }
      buffer_sptr b(w->read());
      buffer_info_sptr info(b->info());
      const uint64_t uS_PER_TICK = UINT64_C(1000000);
      uint64_t tick = info->timestamp_wallclock() + uS_PER_TICK;
      for(b; b = w->read();){
         // is it time to print results yet?
         info = b->info();
         uint64_t timestamp = info->timestamp_wallclock();
         for(; tick <= timestamp; tick += uS_PER_TICK) {
            m->compute(tick, uS_PER_TICK);
            cout << "Time: " << tick / uS_PER_TICK << endl;
            cout << *m << endl;
            m->reset();
         }
         // update metric with frame
         m->add(b);
      }
   } catch(const error& x) {
      cerr << x.what() << endl;
   } catch(const std::exception& x) {
      cerr << x.what() << endl;
   } catch(...) {
      cerr << "unhandled exception!" << endl;
   }
   exit(EXIT_FAILURE);
}
