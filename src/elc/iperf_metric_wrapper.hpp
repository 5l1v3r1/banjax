/* -*- Mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2011 NICTA
 *
 */

#ifndef METRICS_IPERF_METRIC_WRAPPER_HPP
#define METRICS_IPERF_METRIC_WRAPPER_HPP

#include <metric.hpp>
#include <net/encoding.hpp>
#include <net/eui_48.hpp>
#include <net/buffer.hpp>

#include <boost/shared_ptr.hpp>

namespace metrics {

   /**
    * iperf_metric_wrapper is a metric decorator that passes traffic to
    * the decorated metric only if that traffic is generated by iperf.
    */
   class iperf_metric_wrapper : public metric {
   public:

      /**
       * iperf_metric_wrapper constructor.
       *
       * \param wrapped_metric A non-null pointer to the wrapped metric.
       */
      explicit iperf_metric_wrapper(metric_sptr wrapped_metric);

      /**
       * iperf_metric_wrapper copy constuctor.
       *
       * \param other The other iperf_metric_wrapper to initialize from.
       */
      iperf_metric_wrapper(const iperf_metric_wrapper& other);

      /**
       * iperf_metric_wrapper assignment operator.
       *
       * \param other The other iperf_metric_wrapper to assign from.
       * \return A reference to this iperf_metric_wrapper.
       */
      iperf_metric_wrapper& operator=(const iperf_metric_wrapper& other);

      /**
       * iperf_metric_wrapper destructor.
       */
      virtual ~iperf_metric_wrapper();

      /**
       * Add a frame to the iperf_metric_wrapper and update the iperf_metric_wrapper statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Return a pointer to a clone (deep copy) of this iperf_metric_wrapper
       * instance. The clone is allocated on the heap using new and
       * the caller is responsible for ensuring it is deleted.
       *
       * \return A poiner to a new iperf_metric_wrapper instance.
       */
      virtual iperf_metric_wrapper *clone() const;

      /**
       * Compute the metric.
       *
       * \param time The 64 bit MAC time for the end of the time period.
       * \param delta_us The time (in microseconds) since the start of the time period.
       * \return The value of this metric as a double.
       */
      virtual double compute(uint64_t mactime, uint32_t delta_us);

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
       * Pointer to the wrapped metric.
       */
      metric_sptr wrapped_metric_;

   };

}

#endif // METRICS_IPERF_METRIC_WRAPPER_HPP
