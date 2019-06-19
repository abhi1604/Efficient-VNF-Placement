# Efficient-VNF-Placement
Efficient VNF Placement


To run the simulation, follow the below steps:

* g++ min_path.cpp -std=c++14
* ./a.out NUM_REQUESTS


*NUM_REQUESTS is the number of requests that is given to the algorithm*

In the repo scripts are present to automate the testing process and analyze the collected logs to give the final results

Run:

* bash automation.sh | tee stats.logs
* python3 log_processing.py


To check taking into consideration shareability, modify the is_shareable function in params.h

For shareable:

	if(id==0||id==1)
		return true;
	else
		return false;

For non-shareable:

	if(id==0||id==1)
		return false;
	else
		return false;


For changing the delays and other parameters for the request, change the respective parameters in params.h

For example:
// Request delay
int REQUEST_MIN_DELAY;
int REQUEST_MAX_DELAY;

This work is an extension of the following paper:

Zhang, Qixia et al. “Adaptive Interference-Aware VNF Placement for Service-Customized 5G Network Slices.” (2019).

Link of the paper: https://www.semanticscholar.org/paper/Adaptive-Interference-Aware-VNF-Placement-for-5G-Zhang-Liu/5aeae87f05bb4a5f08b9fae9a135863c4eaeea59

The code is available at: https://github.com/abhi1604/Efficient-VNF-Placement
