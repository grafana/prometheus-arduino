# PrometheusArduino

An Arduino library for sending prometheus metrics directly to a Prometheus remote write endpoint.


## Overview

Prometheus is perhaps most famous for having a pull/scrape type ingest model however this sometimes doesn't work very well for embeded devices like Arduinos.

This library provides the ability to push samples directly to any Prometheus Remote Write endpoint and thus includes not only Prometheus but many other Prometheus Remote Write storage systems such as [Cortex](https://cortexmetrics.io/) and [Thanos](https://thanos.io/).

Sending remote write samples to Prometheus itself is a fairly new feature and is disabled by default, see [the Prometheus docs](https://prometheus.io/docs/prometheus/latest/disabled_features/#remote-write-receiver) for info on enabling.


## TimeSeries

If you are familiar with Prometheus you are familiar with a Series, if not Prometheus defines a Series as 

```
Prometheus fundamentally stores all data as time series: streams of timestamped values belonging to the same metric and the same set of labeled dimensions.
```

A series (or TimeSeries in this library) is defined by a name and the set of labels

Series are constructed with the follwing information:

```
TimeSeries(batchSize, metricName, labelset, numberLabels)
```

There is a section below on [batching](#batching) and the batchSize parameter.

Example with no labels:

```C
TimeSeries ts1(5, "ambient_temp_celsius", {}, 0);
```

Example with labels:

```C
TimeSeries ts1(5, "ambient_temp_celsius", {{ "job", "esp32-test" }, {"host","esp32"}}, 2);
```

**NOTE: Please be sure to update the last parameter to match the number of labels in your definition.**

## Metric Names

Start with the [Prometheus Docs](https://prometheus.io/docs/concepts/data_model/#metric-names-and-labels) for metric and label names.  Also check out the [best practices](https://prometheus.io/docs/practices/naming/) page too.

This will help you understand good metric names, but you'll want to read the section below for more information on  _Gauges, Counters, and Histograms_.


## Labels

Prometheus labels are used to add dimensions to your metrics, for example you might have a metric named `uptime_milliseconds_total` which is a counter for an Arduinos uptime (you can see this in the examples).

If you had many Arduinos sending in data, you wouldn't want to define a separate metric for each of them rather you would want to identify them with labels.

Labels are key=value pairs and more info can be found about them [in the Prometheus docs](https://prometheus.io/docs/practices/naming/#labels)

Every new label value defines a new series (TimeSeries) and as such are defined separately in this library:

```C
TimeSeries ts1(5, "ambient_temp_celsius", {{ "job", "esp32-test" }, {"host","esp32-lr"}, {"location","livingroom"}}, 3);
TimeSeries ts2(5, "ambient_temp_celsius", {{ "job", "esp32-test" }, {"host","esp32-br"}, {"location","basement"}}, 3);
```

Labels can be used at query time to select some or all of your series

```
ambient_temp_celsius{location="livingroom"}  # Show just livingroom
ambient_temp_celsius{job="esp32-test"}       # Show both series
```


## Gauges, Counters, Histograms, and Summaries

This library currently doesn't have any higher level concepts for Gauges, Counters, or Histograms, however interestingly Prometheus does not actually differentiate between these in its storage.  That is to say they are all the same to Prometheus anyway.

Gauges, Counters, and Histograms are defined based on some specific naming semantics as well as behaviors, as a result you can easily create these yourself.

### Gauge

A gauge is the most straightforward, assign any name you would like as long as you don't end it with `_total`, `_bucket`, `_sum`, `_count` as these are specific to the other types.

The value for a gauge can be set to anything at any time, it can go up or down with each sample.

Read the [Prometheus Docs](https://prometheus.io/docs/concepts/metric_types/#gauge) for more information.

### Counter

A counter is essentially a gauge with restrictions:

* The name must end in `_total` (e.g. `uptime_milliseconds_total`)
* The value must never decrease unless rest to zero.

A counter value can be increased by any sum, but it must never decrease _unless_ it resets completely to zero.

Counter resets are understood by prometheus and when you query across counter resets with `rate` or `increase` functions those resets will be automatically handled and removed.

Read the [Prometheus Docs](https://prometheus.io/docs/concepts/metric_types/#counter) for more information.

#### Histogram

A histogram in Prometheus is a set of counters with specific naming suffixes and some special labels.

Implementing a histogram in this library would take some work and hopefully someday this work will be included directly in the library.

Start with the [Prometheus Docs](https://prometheus.io/docs/concepts/metric_types/#histogram) to get a better understanding.

To implement a histogram you would need to define several series

```C
TimeSeries h1b1(5, "http_request_duration_seconds_bucket", {{ "job", "esp32-test" }, {"le","1"}}, 2);
TimeSeries h1b2(5, "http_request_duration_seconds_bucket", {{ "job", "esp32-test" }, {"le","5"}}, 2);
TimeSeries h1b3(5, "http_request_duration_seconds_bucket", {{ "job", "esp32-test" }, {"le","+Inf"}}, 2);
TimeSeries h1c(5, "http_request_duration_seconds_count", {{ "job", "esp32-test" }}, 1);
TimeSeries h1s(5, "http_request_duration_seconds_sum", {{ "job", "esp32-test" }}, 1);
// Create counters to store the count value.
uint32_t b1,b2,b3,c,s;
```

the `le` label is special and stands for `less than or equal`, the `+Inf` bucket is also special and should always be included for any values that don't go into the other buckets.

Now for how it works. For any value you want to `observe` you need to increment every counter which is less than or equal to the value as well as increment the total count and add the value to the sum:

Example to `observe` the value 3:
```
b2++;
b3++;
c++;
s+=3;
int64_t ts = getTimeMillis();
h1b2.addSample(ts, b2);
h1b3.addSample(ts, b3);
h1c.addSample(ts, c);
h1s.addSample(ts, s);
```
Notice we did not increment b1 or send a samle to h1b1 because 3 is not less than or equal to the `le` of 1 for bucket 1.

If we were to `observe` the value 10:
```
b3++;
c++;
s+=10;
int64_t ts = getTimeMillis();
h1b3.addSample(ts, b3);
h1c.addSample(ts, c);
h1s.addSample(ts, s);
```

Note, I didn't have a chance to test this yet so hopefully this is right :)


### Summaries

TIL Prometheus has a Summary type, will return to add an example.


## Examples

* prom_01_local will demonstrate how to setup a simple example to a local Prometheus or other remote write endpoint with no auth and no SSL.
* prom_02_grafana_cloud demonstrates how to connect to an endpoint which uses authentication and SSL, check out the [Grafana Cloud](https://grafana.com/products/cloud/) free tier to get started with this example today.

## SSL

TLS/SSL configuration is difficult with Arduinos as the hardware is barely powerful enough (or not powerful enough at all in some cases) to do SSL negotiations, and there are several types of hardware and software libraries for doing SSL.

For most devices the [SSLClient library](https://github.com/OPEnSLab-OSU/SSLClient) is used, which is based on BearSSL but seemed to be more resource efficient in testing.

To use SSL you need to create a `certificates.h` file thankfully the wonderful SSLClient library provides some documentation and a number of ways to make this fairly easy: [check out the project docs](https://github.com/OPEnSLab-OSU/SSLClient/blob/master/TrustAnchors.md#generating-trust-anchors)


## Batching

HTTP requests have a lot of overhead and if you are generating a lot of metrics it's much more efficient to batch them.

When defining a TimeSeries you must specify a batch size, this then allocates memory on the heap for storing batches.

You do not need to fill a batch to send it.

It is not required to batch, but it will improve efficieny a little and reduce bandwidth consumption.

**NOTE** You must reset the batch after every send with the `resetSamples()` function.

## Buffers

There are 2 important buffers used to serialize the data to send to Prometheus, one for creating the proto and one for Snappy compressing it.

Unfortunately it's not possible to use the same buffer for both, as you need to read the proto buffer to create the Snappy compressed output in a new buffer.

**By default this library sets both of these buffers to 512 bytes.**

The size for both of these buffers can be set when initializing the write request object

```
WriteRequest req(2, 512);
```

These buffers are allocated on the stack during a send, meaning you need 2x this buffer size value in stack space to send.

It's possible, but difficult to estimate how big the buffer needs to be, mainly because protos do a good job of reducing the size of numeric values.

For most devices the "free memory" is printed out during this process, you can use this to understand if you have enough stack space to perform a send without crashing your stack. _Make sure you have as much free mem before starting to serialize as 2x the buffer size._

## How to size the buffer

Enable debug logging on your write request, 

```
req.setDebug(Serial);
```

Send your data and look at the logs, if you get an error on `toSnappyProto` and you see `stream full` in the debug output then the buffer is undersized for serializing to protobuf, try increasing it:

```
WriteRequest req(2,1024);
```

Compression can actually result in a larger object in some cases, usually around small objects with a lot of strings.

The debug logging will tell you the size needed for compression:

```
Required buffer size for compression: 754
```

Make sure your buffer size is bigger than this.

If you are working on a very memory constrained device, keep your metric names short and your labels few and names short as well.



## License Notes

This library ports 2 existing libraries internally as there were not Arduino versions of these libraries already:

* [nanopb](https://github.com/nanopb/nanopb) which is licensed under the [zlib license](https://github.com/grafana/prometheus-arduino/blob/main/src/proto/LICENSE) and code is found in the `src/proto` directory.
* [snappy-c](https://github.com/andikleen/snappy-c) which is licensed under an [custom license](https://github.com/grafana/prometheus-arduino/blob/main/src/snappy/LICENSE) and code is found in the `src/snappy` directory.

Some small modifications were made to both sources to get them to compile and work on Arduino and are tagged with `// E.Welch` 