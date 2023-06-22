#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>
#include <chrono>
using namespace std::chrono;

#include "tensorflow/c/c_api.h"


int main(int argc, const char * argv[])
{
  srand (time(NULL));
  
  
  std::string hello = "Hello from TenforFlow ";
  hello += TF_Version();
  
  printf("%s\n", hello.c_str());
  
  FILE *fp_graph = fopen(argv[1], "rb");
  long graph_size = 0;
  char *graph_data;
  if (fp_graph) {
    fseek(fp_graph, 0, SEEK_END);
    graph_size = ftell(fp_graph);
    fseek(fp_graph, 0, SEEK_SET);
    graph_data = new char[graph_size];
    fread(graph_data, graph_size, 1, fp_graph);
    fclose(fp_graph);
  }
  else {
    printf("Failed to open dnn file.\n");
    return -1;
  }
  
  TF_Graph *graph = TF_NewGraph();
  
  TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();
  
  //char* cprefix = "";
  //TF_ImportGraphDefOptionsSetPrefix(opts, cprefix);
  
  TF_Buffer* buf =
  TF_NewBufferFromString(graph_data, graph_size);
  TF_Status* status = TF_NewStatus();
  
  TF_GraphImportGraphDef(graph, buf, opts, status);
  TF_Code tf_code = TF_GetCode(status);
  if (tf_code != TF_OK) {
    printf("TF_GraphImportGraphDef failed: %d\n", tf_code);
  }
  TF_DeleteImportGraphDefOptions(opts);
  TF_DeleteBuffer(buf);
  
  
  TF_SessionOptions* session_opts = TF_NewSessionOptions();
  TF_Session* session = TF_NewSession(graph, session_opts, status);
  if (tf_code != TF_OK) {
    printf("TF_NewSession failed: %d\n", tf_code);
  }
  TF_DeleteSessionOptions(session_opts);
  
  
  int ninputs = 1;
  int noutputs = 1;
  int ntargets = 1;
  std::unique_ptr<TF_Output[]> inputs(new TF_Output[ninputs]);
  std::unique_ptr<TF_Tensor* []> input_values(new TF_Tensor*[ninputs]);
  std::unique_ptr<TF_Output[]> outputs(new TF_Output[noutputs]);
  std::unique_ptr<TF_Tensor* []> output_values(new TF_Tensor*[noutputs]);
  std::unique_ptr<TF_Operation* []> targets(new TF_Operation*[ntargets]);
  
  TF_Operation *oper_input = TF_GraphOperationByName(graph, "features");
  inputs[0] = TF_Output{oper_input, 0};
  
  const int batch_size = 4;
  const int batch_num = 25*10;
  
  const int input_dim = batch_size * 957;
  int64_t input_dims[] = {input_dim};
  TF_Tensor *tensor_input = TF_AllocateTensor(TF_FLOAT, input_dims,
                                              sizeof(input_dims)/sizeof(int64_t), input_dim * sizeof(float));
  float *tensor_input_data = (float*)TF_TensorData(tensor_input);
  for (int i=0; i<input_dims[0]; i++) {
    tensor_input_data[i] = (rand() / (float)RAND_MAX) * 2 - 1;
  }
  input_values[0] = tensor_input;
  
  TF_Operation *oper_output = TF_GraphOperationByName(graph, "postprobs");
  outputs[0] = TF_Output{oper_output, 0};
  
  const int output_dim = batch_size * 5191;
  int64_t output_dims[] = {output_dim};
  TF_Tensor *tensor_output = TF_AllocateTensor(TF_FLOAT, output_dims,
                                               sizeof(output_dims)/sizeof(int64_t), output_dim * sizeof(float));
  output_values[0] = tensor_output;
  
  targets[0] = oper_output;
  
  milliseconds start_ms = duration_cast< milliseconds >(
                                                        system_clock::now().time_since_epoch());
  for (int i=0; i<batch_num; i++) {
    TF_SessionRun(session, nullptr, inputs.get(), input_values.get(),
                  static_cast<int>(ninputs), outputs.get(), output_values.get(),
                  static_cast<int>(noutputs), targets.get(),
                  static_cast<int>(ntargets), nullptr, status);
    if (tf_code != TF_OK) {
      printf("TF_SessionRun failed: %d\n", tf_code);
    }
  }
  milliseconds stop_ms = duration_cast< milliseconds >(
                                                       system_clock::now().time_since_epoch());
  printf("time cost: %lld\n", stop_ms.count() - start_ms.count());
  
  TF_CloseSession(session, status);
  TF_DeleteSession(session, status);
  TF_DeleteGraph(graph);
  TF_DeleteStatus(status);
  return 0;  
}  
