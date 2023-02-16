#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define pwrtwo(x) ( 1 << (x))

int main(int argc, char* argv[]){
    
    FILE *fp1, *fp2;
    int file_size;
    int generator_len, dataword_len, codeword_len;
    int dataword_cnt;
    int padding_bit_num, out_buf_size, out_buf_idx;
    char generator[8], data[8], input;
    char *out_buf;

    int cnt = 0, byte_cnt;
    
    // usage error
    if (argc != 5) {
        printf("usage: ./crc_encoder input_file output_file generator dataword_size\n");
        return 1;
    }

    // input file open error
    fp1 = fopen(argv[1], "r");
    if (fp1 == NULL) {
        printf("input file open error.\n");
        fclose(fp1);
        return 1;
    }

    // calculate file size
    // move file pointer from EOF to front with 0 byte offset
    // ftell - tell current position of file pointer in stream
    fseek(fp1, 0, SEEK_END);
    file_size = ftell(fp1); // example file size in byte = 42(char + line feed)
    rewind(fp1);

    // output file open error
    fp2 = fopen(argv[2], "w");
    if (fp2 == NULL) {
        printf("output file open error.\n");
        fclose(fp1); fclose(fp2);
        return 1;
    }

    // dataword size error
    dataword_len = argv[4][0] - 48;
    if (dataword_len != 4 && dataword_len != 8) {
        printf("dataword size must be 4 or 8.\n");
        fclose(fp1); fclose(fp2);
        return 1;
    }

    // generator setting
        // strcpy includes null value
        // strlen do not includes null value
    strcpy(generator, argv[3]);
    generator_len = strlen(generator);
    for (int i=0 ; i<generator_len ; i++) {
        generator[i] -= '0';
    }

    // variables setting
    codeword_len = dataword_len + generator_len - 1;
    dataword_cnt = file_size * 8 / dataword_len;

    // init padding bit and output buffer
    padding_bit_num = 8 - (dataword_cnt * codeword_len % 8);
    out_buf_size = 8 + padding_bit_num + (dataword_cnt * codeword_len);
    out_buf = (char*)malloc(sizeof(char) * out_buf_size);
    memset(out_buf, 0, out_buf_size);
    
    // put padding bit info in output buffer
    int temp = padding_bit_num;
    for (int i=4, j=0 ; i>=1 ; i/=2, j++) {
        if (temp/i) {
            out_buf[5+j] = 1;
            temp -= i;
        }
    }
    out_buf_idx = 8 + padding_bit_num;

    // start reading input
    for (int r = 0 ; r < file_size ; r++) {
        
        // read one letter and put it in data array
        fread(&input, 1, 1, fp1);
        for (int i=0 ; i<8 ; i++)
            data[7-i] = (input >> i) & 1;
        
        // set variables
        char* codeword, *dividend, *quotient, *remainder;
        int quotient_len = codeword_len - generator_len + 1;
        
        codeword = (char*)malloc(sizeof(char)*codeword_len);
        dividend = (char*)malloc(sizeof(char)*codeword_len);
        quotient = (char*)malloc(sizeof(char)*quotient_len);
        remainder = (char*)malloc(sizeof(char)*(generator_len - 1));
        
        // dataword length 4
        if (dataword_len == 4) {
            for (int i=0 ; i<2 ; i++) {
                // data init
                memset(dividend, 0, codeword_len);
                memcpy(dividend, data+(i*4), dataword_len);
                memset(codeword, 0, codeword_len);
                memcpy(codeword, data+(i*4), dataword_len);
                
                // divide
                for (int j = 0 ; j < quotient_len ; j++) {
                    if (dividend[j]) { // dividable by generator
                        quotient[j] = 1;
                        
                        for (int k = 0 ; k < generator_len ; k++) {
                            if (generator[k] == dividend[j + k])
                                dividend[j + k] = 0;
                            else
                                dividend[j + k] = 1;
                        }
                    }

                    else { // undividable by generator
                        quotient[j] = 0;
                    }
                }

                // set remainder
                for (int j = generator_len - 2, k = codeword_len - 1 ; j >= 0 ; j--, k--) {
                    remainder[j] = dividend[k];
                }
                
                // set codeword
                memcpy(codeword + dataword_len, remainder, generator_len-1);
                
                
                // set output buffer
                for (int j=0 ; j<codeword_len ; j++) {
                    out_buf[out_buf_idx++] = codeword[j];
                }
                
                /*               
                // --------------debugging-------------------
                // ------------------------------------------ 
                // input
                printf("input char : %c - ", input);
                
                // data
                for (int j=0 ; j<8 ; j++)
                    printf("%d", data[j]);
                printf("\n");    
                
                // divided data by dataword length
                for (int j=0 ; j<4 ; j++) {
                    printf("%d", data[i * 4 + j]);
                }
                printf(" divided by generator\n");
                
                // quotient result
                printf("quotient result : ");
                for (int j=0 ; j<generator_len ; j++)
                    printf("%d", quotient[j]);
                printf("\n");

                // remainder result
                printf("remainder result : ");
                for (int j=0 ; j<generator_len-1 ; j++){
                    printf("%d", remainder[j]);
                }
                printf("\n");

                // codeword result
                printf("codeword result : ");
                for (int j=0 ; j<7 ; j++)
                    printf("%d", codeword[j]);

                printf("\n\n");
                
                // ------------------------------------------
                // ------------------------------------------
                */
            }

        }

        // dataword length 8
        else if (dataword_len == 8) {
            // init
            memset(dividend, 0, codeword_len);
            memcpy(dividend, data, dataword_len);
            memset(codeword, 0, codeword_len);
            memcpy(codeword, data, dataword_len);

            // divide
            for (int i=0 ; i < quotient_len ; i++) {
                if (dividend[i]) {
                    quotient[i] = 1;
                    for (int j=0 ; j<generator_len ; j++) {
                        if (generator[j] == dividend[i+j])
                            dividend[i+j] = 0;
                        else
                            dividend[i+j] = 1;
                    }
                }
                else {
                    quotient[i] = 0;
                }
            }

            // set remainder
            for (int i=generator_len-2, j=codeword_len-1 ; i>=0 ; i--, j--)
                remainder[i] = dividend[j];

            // set codeword
            memcpy(codeword + dataword_len, remainder, generator_len - 1);

            // set output buffer
            for (int i=0 ; i<codeword_len ; i++)
                out_buf[out_buf_idx++] = codeword[i];
        }

        free(codeword);
        free(dividend);
        free(quotient);
        free(remainder);
    }
    
    /*
    //------------------------debugging---------------------
    int cnt_tmp=0;
    for (int i=0 ; i<out_buf_size ; i++){
        if (cnt_tmp%8 == 0)
            printf("%d ", cnt_tmp);
        printf("%d", out_buf[i]);
        if (cnt_tmp%8==7)
            printf("\n");
        cnt_tmp++;
    }
    printf("\n");
    //------------------------------------------------------
    */
    
    // out_buf array is char array which each element has value 0 or 1. 
    // i.e) it is "codewords" in binary form
    // make it to byte form
    // and write result to output file
    byte_cnt = out_buf_size / 8;
    unsigned char* code_stream;
    code_stream = (unsigned char*)malloc(sizeof(unsigned char)*byte_cnt);
    memset(code_stream, 0, byte_cnt);
    for (int i=0 ; i<byte_cnt ; i++) {
        for (int j=0 ; j<8 ; j++) {
            code_stream[i] += out_buf[8 * i + j] * pwrtwo(7-j);
        }
    }

    fwrite(code_stream, byte_cnt, 1, fp2);
    
    free(out_buf);
    free(code_stream);
    fclose(fp1);
    fclose(fp2);
    return 0;
}
