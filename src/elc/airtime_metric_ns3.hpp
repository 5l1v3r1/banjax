/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_AIRTIME_METRIC_NS3_HPP
#define METRICS_AIRTIME_METRIC_NS3_HPP

#include <abstract_metric.hpp>
#include <net/encoding.hpp>
#include <map>

namespace metrics {

   /**
    * airtime_metric_ns3 is defined by 802.11s as its default routing
    * metric (see IEEE 802.11s-d8 s 11A.7).
    *
    * This version is based on the metric found in the NS-3 network
    * simulator implementation of 802.11.
    */
   class airtime_metric_ns3 : public abstract_metric {
   public:

      /**
       * airtime_metric_ns3 constructor.
       *
       * \param enc A non-null pointer to the encoding.
       * \param rts_cts_threshold Use RTS/CTS when rts_cts_threshold <= test frame size
       */
      airtime_metric_ns3(net::encoding_sptr enc, uint16_t rts_cts_threshold);

      /**
       * airtime_metric_ns3 copy constuctor.
       *
       * \param other The other airtime_metric_ns3 to initialize from.
       */
      airtime_metric_ns3(const airtime_metric_ns3& other);

      /**
       * airtime_metric_ns3 assignment operator.
       *
       * \param other The other airtime_metric_ns3 to assign from.
       * \return A reference to this airtime_metric_ns3.
       */
      airtime_metric_ns3& operator=(const airtime_metric_ns3& other);

      /**
       * airtime_metric_ns3 destructor.
       */
     virtual ~airtime_metric_ns3();

      /**
       * Add a frame to the airtime_metric_ns3 and update the airtime_metric_ns3 statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this airtime_metric_ns3
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A pointer to a new airtime_metric_ns3 instance.
       */
      virtual airtime_metric_ns3 *clone() const;

      /**
       * Compute the metric.
       *
       * \param delta_us The time (in microseconds) over which to compute the metric.
       * \return The value of this metric as a double.
       */
      virtual double compute(uint32_t delta_us);

      /**
       * Reset the internal state of the metric.
       */
      virtual void reset();

      /**
       * Write this object in human-readable form to ostream os.
       *
       * \param os A reference to the stream to write to.
       */
      virtual void write(std::ostream& os) const;

   private:

      /**
       * Scan the standard rateset of the default encoding and return
       * the value which has the smallest difference to r.
       *
       * \param r The rate to find.
       * \return The value in rates which is closest to r.
       */
      uint32_t closest_rate(uint32_t r) const;

   private:

      /**
       * Pointer to the default channel encoding.
       */
      net::encoding_sptr enc_;

      /**
       * The RTS/CTS threshold.
       */
      uint16_t rts_cts_threshold_;

      /**
       * The total number of successfully delivered frames.
       */
      uint32_t frames_;

      /**
       * The total number of successfully delivered packets.
       */
      uint32_t packets_;

      /**
       * The total number of IP payload octets successfully delivered.
       */
      uint_least32_t packet_octets_;

      /**
       * Sum of the data rates used to send packets (used to compute average).
       */
      uint_least32_t rates_Kbs_sum_;

      /**
       * Total number of frames successfully delivered at a given rate.
       */
      std::map<uint32_t, uint32_t> frames_rate_;

      /**
       * Total number of packets successfully delivered at a given rate.
       */
      std::map<uint32_t, uint32_t> packets_rate_;

      /**
       * Stashed value of this metric.
       */
      double airtime_;

   };

}

#endif // METRICS_AIRTIME_METRIC_NS3_HPP