#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define pwrtwo(x) ( 1 << (x) )

int main(int argc, char* argv[]){
    FILE *fp_in, *fp_out, *fp_res;
    int file_size, dataword_len, generator_len, codeword_len;
    int padding_bit_num, codeword_cnt = 0, byte_cnt = 0;
    char generator[8];
    unsigned char *in_buf, *data_buf, *out_buf;
    unsigned char *codeword, *dataword;
    int in_buf_idx, data_buf_idx, out_buf_idx, err_cnt = 0;

    // usage error
    if (argc != 6) {
        printf("usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
        return 1;
    }

    // input file open error
    fp_in = fopen(argv[1], "r");
    if (fp_in == NULL) {
        printf("input file open error.\n");
        return 1;
    }

    // calculate file size
    fseek(fp_in, 0, SEEK_END);
    file_size = ftell(fp_in);
    rewind(fp_in);

    // output file open error
    fp_out = fopen(argv[2], "w");
    if (!fp_out) {
        printf("output file open error.\n");
        return 1;
    }

    // result file open error
    fp_res = fopen(argv[3], "w");
    if (!fp_res) {
        printf("result file open error.\n");
        return 1;
    }
    
    // dataword size error
    dataword_len = argv[5][0] - '0';
    if (dataword_len != 4 && dataword_len != 8) {
        printf("dataword size must be 4 or 8.\n");
        return 1;
    }

    // generator setting
    strcpy(generator, argv[4]);
    generator_len = strlen(generator);
    for (int i=0 ; i<generator_len ; i++)
        generator[i] -= '0';

    // read input and save it in buffer in binary form
    unsigned char* code_stream;
    code_stream = (unsigned char*)malloc(sizeof(unsigned char)*file_size); 
    fread(code_stream, 1, file_size, fp_in);
   
    in_buf = (unsigned char*)malloc(sizeof(unsigned char)*file_size*8);
    for (int i=0 ; i<file_size; i++) {
        //printf("%d ", 8*i);
        for (int j=0 ; j<8 ; j++) {
            in_buf[8 * i + j] = code_stream[i] & pwrtwo(7-j);
            in_buf[8 * i + j] /= pwrtwo(7-j);
            //printf("%d", in_buf[8*i+j]);
        }
        //printf("\n");
    }
    free(code_stream);


    // read first byte which implies padding bit num
    padding_bit_num = 0;
    for (int i=0 ; i<3 ; i++) {
        padding_bit_num += pwrtwo(i) * in_buf[7-i];
    }
    
    // other variables setting
    codeword_len = dataword_len + generator_len - 1;
    codeword_cnt = (file_size * 8 - 8 - padding_bit_num) / codeword_len;
    codeword = (unsigned char*)malloc(sizeof(unsigned char)*codeword_len);
    if (dataword_len == 4)
        byte_cnt = codeword_cnt/2;
    else
        byte_cnt = codeword_cnt;

    // remove padding bit
    in_buf_idx = 8 + padding_bit_num;
    
    // set data buffer which will save dataword from codeword
    data_buf = (unsigned char*)malloc(sizeof(unsigned char)* codeword_cnt * dataword_len);
    memset(data_buf, 0, codeword_cnt * dataword_len);
    data_buf_idx = 0;

    // read all codewords
    for (int r=0 ; r<codeword_cnt ; r++) {
        
        // copy one codeword from buffer
        memcpy(codeword, in_buf + in_buf_idx, codeword_len);

        // save dataword from codeword
        for (int i=0 ; i<dataword_len ; i++) {
            data_buf[data_buf_idx+i] = codeword[i];
        }
        data_buf_idx += dataword_len;
        
        // division
        int quotient_len = codeword_len - generator_len + 1;
        for (int i=0 ; i<quotient_len ; i++) {
            if (codeword[i]) {
                for (int j=0 ; j<generator_len ; j++) {
                    if (codeword[i+j] == generator[j])
                        codeword[i+j] = 0;
                    else
                        codeword[i+j] = 1;
                }
            }
        }
        

        // error check
        for (int i = 0 ; i < generator_len - 1 ; i++) {
            if (codeword[codeword_len - 1 - i]) {
                err_cnt++;
                break;
            }
        }

        in_buf_idx += codeword_len;
    }

    out_buf = (unsigned char*)malloc(sizeof(unsigned char) * byte_cnt);
    memset(out_buf, 0, byte_cnt);
    out_buf_idx = 0;
    for (int i=0 ; i<byte_cnt ; i++){
        for (int j=0 ; j<8 ; j++) {
            out_buf[out_buf_idx] += data_buf[8*i+j] * pwrtwo(7-j);
        }
        out_buf_idx++;
    }

    // file write
    fprintf(fp_res, "%d %d\n", codeword_cnt, err_cnt);
    fwrite(out_buf, byte_cnt, 1, fp_out);

    // deallocation
    free(in_buf);
    free(out_buf);
    free(data_buf);
    free(codeword);

    // file close
    fclose(fp_in);
    fclose(fp_out);
    fclose(fp_res);

        
    return 0;
}
