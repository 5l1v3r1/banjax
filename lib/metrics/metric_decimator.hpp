/* -*- mode: C++; tab-width: 3; -*- */

/*
 * Copyright 2012 NICTA
 *
 */

#ifndef METRICS_METRIC_DECIMATOR_HPP
#define METRICS_METRIC_DECIMATOR_HPP

#include <metrics/metric.hpp>

#include <deque>
#include <string>

namespace metrics {

   /**
    * metric_decimator decimates the input so only 1/n packets are
    * used to compute the metric.
    */
   class metric_decimator : public metric {
   public:

      /**
       * metric_decimator constructor.
       *
       * \param name The name of the damped metric.
       * \param metric Pointer to the metric to damp.
       * \param n The decimation constant.
       */
      metric_decimator(std::string name, metric_sptr metric, uint16_t n);

      /**
       * metric_decimator copy constructor.
       *
       * \param other The other metric_decimator to initialize from.
       */
      metric_decimator(const metric_decimator& other);

      /**
       * metric_decimator assignment operator.
       *
       * \param other The other metric_decimator to initialize from.
       * \return A reference to this metric_decimator.
       */
      metric_decimator& operator=(const metric_decimator& other);

      /**
       * metric_decimator destructor.
       */
      virtual ~metric_decimator();

      /**
       * Return a pointer to a clone (a deep copy) of this
       * metric_decimator instance. The clone is allocated on the heap
       * using new and the caller is responsible for ensuring it is
       * deleted.
       *
       * \return A poiner to a new metric_decimator instance.
       */
      virtual metric_decimator *clone() const;

      /**
       * Add a frame to the metric and update the metric statistics.
       *
       * \param b A shared_pointer to the buffer containing the frame.
       */
      virtual void add(net::buffer_sptr b);

      /**
       * Compute the metric. Subclasses must implement either this
       * method or the comput(uint32_t) version. Failure to do so will
       * result in a logic_error being thrown at runtime.
       *
       * \param mactime The 64 bit MAC time for the end of the time period.
       * \param delta_us The time (in microseconds) since we last computed the metric.
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
       * Name of the damped metric.
       */
      std::string name_;

      /**
       * The metric being damped.
       */
      metric_sptr metric_;

      /**
       * Discard (n-1) from every n packets.
       */
      uint32_t n_;

      /**
       * Count of packets seen so far.
       */
      uint32_t i_;

      /**
       * The stashed value of this metric.
       */
      double value_;
   };

}

#endif // METRICS_METRIC_DECIMATOR_HPP
