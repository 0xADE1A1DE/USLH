This folder is for testing if the processor is vulnerable to variable timing instructions under the speculation.  


## How to run it.
**You will need to manually set `$val` in the run.bash to run the test with fast value or slow value.**

Simply run `python3 test.py $pair_max $pair_min`.  
*pair_max* is the maximum number of pairs to test while *pair_min* is the minimum value to test.  
An example:  
`python3 test.py 55 40`

And you will see result similar to:  
```
  40  16.51
  41  15.13
  42  15.19
  43  16.34
  44  17.36
  45  16.27
  46  20.32
  47  24.88
  48  44.87
  49  79.16
  50  131.29
  51  140.01
  52  161.79
  53  145.44
  54  140.36
  55  138.3
```

It indicates that 50 pairs of variable time instructions on fast value can lock RS during the speculation. 


For slow value, you may see results similar to the below:
```
  40  20.73
  41  17.49
  42  16.26
  43  34.02
  44  36.88
  45  115.04
  46  144.05
  47  144.0
  48  140.51
  49  132.11
  50  137.25
  51  133.74
  52  143.29
  53  131.1
  54  128.64
  55  144.5
```

which indicates that with 45 pairs variable time instructions on a slow value can lock RS during the speculation.
