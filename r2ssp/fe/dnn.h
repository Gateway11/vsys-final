
/*
	class DNN written by Zejun MA, 2012.05.12
*/

#ifndef _DEEP_NN_
#define _DEEP_NN_

// C libs
#include <stdio.h>
#include <string.h>
#include <assert.h>

// C++ libs
#include <string>
#include <iostream>

enum NetOutType {LINEAR, SIGMOID, SOFTMAX};

class Layer
{
public:
	Layer()
	{
		curr_layer_num = 0;
		prev_layer_num = 0;
		curr_layer_num_stride = 0;
		prev_layer_num_stride = 0;
		weights = NULL;
		bias = NULL;
		activations = NULL;
	}
	~Layer();

	int prev_layer_num;			// neuron number of previous layer
	int prev_layer_num_stride;	// actual allocated neuron number of previous layer, for aligned matrix
	int curr_layer_num;			// neuron number of current layer
	int curr_layer_num_stride;	// actual allocated neuron number of current layer, for aligned bias and activation
	float *weights;				// weight matrix between current and next layer
	float *bias;				// bias vector of current layer
	float *activations;			// activation of current layer 
};


class CDNN
{
public:

	CDNN();
	~CDNN();

	// I/O function
	// load weight matrix according to curr * prev
	bool load_model_org(const char* modelfile, int tgtid);

	// load weight matrix according to prev * curr
	bool load_model_batch(const char* modelfile, int tgtid);

	void print_model_org(const char* outmod) const; // for debug use

	void print_model_batch(const char* outmod) const; // for debug use
	
	// set activation of each layer to zero
	void resetNetAct();

	// check model status before let it work
	inline bool check_model() const
	{
		if(model==NULL || !status)
			return false;
		else
			return true;
	}

	// visit from 2nd to final layer's statistics
	inline const Layer& get_network_layer(int layerid) const
	{
		if (!check_model()){
			printf("Error, Empty model, load first");
			exit(-1);
		}
		assert(layerid>=2 && layerid<=total_layer_num);
		return model[layerid-2];
	}

	/* forward computation
		(-) raw version: use sgemv and sse2
		(-) opt1 version: use sgemm
		(-) opt2 version: use fixed-point
	*/
	bool get_batch_output_org(const float* cepbuffer, int frmnum, int frmdim, \
		int& outdim, float* outfea[], NetOutType type);
	bool get_batch_output_batch(const float* cepbuffer, int frmnum, int frmdim, \
		int& outdim, float* outfea[], NetOutType type);
	//bool get_batch_output_opt2(const float* cepbuffer, int frmnum, int frmdim, \
	//	int& outdim, float* outfea[], NetOutType type);

//private:
	int total_layer_num;
	int target_layer_num;
	bool status;
	Layer* model;

	float* prior;
	//float** temp_activations;
	//int maxbatchsize;
};

#endif // _DEEP_NN_
