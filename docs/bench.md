# 性能测试

## echo_server性能测试

测试工具：`ab -n 10000000 -c 1000 -k http://127.0.0.1:{port}/`
本机配置：
```
Name:              AMD Ryzen 7 4800U with Radeon Graphics
Microarchitecture: Zen 2
Technology:        7nm
Max Frequency:     1.800 GHz
Cores:             8 cores (16 threads)
AVX:               AVX,AVX2
FMA:               FMA3
L1i Size:          32KB (256KB Total)
L1d Size:          32KB (256KB Total)
L2 Size:           512KB (4MB Total)
L3 Size:           4MB (8MB Total)
Peak Performance:  460.80 GFLOP/s
```
上面数据来自于`cpufetch`

### this project (single thread):
```
Server Software:
Server Hostname:        127.0.0.1
Server Port:            10009

Document Path:          /
Document Length:        405 bytes

Concurrency Level:      1000
Time taken for tests:   69.095 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      5120000000 bytes
HTML transferred:       4050000000 bytes
Requests per second:    144728.66 [#/sec] (mean)
Time per request:       6.909 [ms] (mean)
Time per request:       0.007 [ms] (mean, across all concurrent requests)
Transfer rate:          72364.33 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      33
Processing:     0    7   0.9      7     214
Waiting:        0    7   0.9      7     214
Total:          0    7   0.9      7     214

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      7
  75%      7
  80%      7
  90%      8
  95%      8
  98%      9
  99%     10
 100%    214 (longest request)
```

### tokio echo_server (single thread):
```
Server Software:
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   70.251 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1060000000 bytes
HTML transferred:       0 bytes
Requests per second:    142347.54 [#/sec] (mean)
Time per request:       7.025 [ms] (mean)
Time per request:       0.007 [ms] (mean, across all concurrent requests)
Transfer rate:          14735.20 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0      13
Processing:     0    7   1.9      7     107
Waiting:        0    7   1.9      7     107
Total:          0    7   1.9      7     114

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      7
  75%      7
  80%      7
  90%      8
  95%      8
  98%      9
  99%     12
 100%    114 (longest request)
```

### muduo echo_server (single thread):
```
Server Software:
Server Hostname:        127.0.0.1
Server Port:            10009

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   69.185 seconds
Complete requests:      10000000
Failed requests:        0
Non-2xx responses:      10000000
Keep-Alive requests:    10000000
Total transferred:      1070000000 bytes
HTML transferred:       0 bytes
Requests per second:    144540.68 [#/sec] (mean)
Time per request:       6.918 [ms] (mean)
Time per request:       0.007 [ms] (mean, across all concurrent requests)
Transfer rate:          15103.37 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      33
Processing:     0    7   0.9      7      33
Waiting:        0    7   0.9      7      21
Total:          0    7   1.0      7      47

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      7
  75%      7
  80%      7
  90%      8
  95%      8
  98%      9
  99%     10
 100%     47 (longest request)
```
