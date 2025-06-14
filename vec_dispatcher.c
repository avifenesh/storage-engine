// Function pointer type for vector operations
typedef float (*vector_op_t)(const float* vec1, const float* vec2, int dim);

// Dispatcher that calls the appropriate operation
float dispatch_operation(const char* op_name, vector_op_t* ops, 
                        const float* vec1, const float* vec2, int dim);