#include    <iostream>
#include    <fstream>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

using namespace std;

typedef union{
    uint8_t data[4];
    uint32_t num;
}word2byte;

static void print_array(void * to_print, uint16_t len)
{
    uint16_t counter;

    for( counter = 0; counter<len;counter++ )
    {
       if( 0 == counter % 16 )
       {
           cout << endl;
       }
        printf( "%02X ", *(((uint8_t *)(to_print)) + counter) );
    }
    printf( "\n" );
}


static uint32_t cy_ota_ble_crc32_update(uint32_t prev_crc32, const uint8_t* buffer, uint16_t buffer_length)
{
    uint32_t crc32 = ~prev_crc32;
    uint16_t i;

    for(i = 0; i < buffer_length; i++)
    {
        uint16_t j;

        crc32 ^= *buffer;
        buffer++;

        for(j = 0; j < 8; j++)
        {
            if(crc32 & (uint32_t)0x1U)
            {
                crc32 = (crc32 >> 1) ^ 0xEDB88320U;
            }
            else
            {
                crc32 = (crc32 >> 1);
            }
        }
    }

    return ~crc32;
}

#include <unistd.h>
extern "C" void transfer_file(int fd, char *filename);
extern "C" int external_main(int fd, char *filename)
//int main()
{
    FILE *pd = NULL;
    uint8_t buf[64];
    uint32_t pre_crc = 0;
    int total_size = 0;
    int error_count = 0;
  
    //pd = fopen("../crab.signed.bin", "rb");
    pd = fopen(filename, "rb");

    while(1)
    {
        int count = fread(buf, sizeof(buf[0]), sizeof(buf), pd);
        if(count == 0)
        {
            break;
        }   

        if(count != 64)
        {
            error_count++;
        }
        total_size += count; //read count
        pre_crc = cy_ota_ble_crc32_update(pre_crc, (uint8_t *)buf, count);
        print_array(buf, count);
        
    }

    cout<<"total size = " << total_size << endl;
    printf("final crc = 0x%x\r\n", pre_crc);
    cout << "error =" << error_count << endl;

    word2byte total_size_num;
    total_size_num.num = total_size;

    word2byte pre_crc_num;
    pre_crc_num.num = pre_crc;

    //step1
    uint8_t prepare_ota[64] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00};
    write(fd, prepare_ota, sizeof(prepare_ota));
    
    //step2
    uint8_t total_size_ota[64] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x11};
    memcpy(&total_size_ota[6], total_size_num.data, 4);
    write(fd, total_size_ota, sizeof(total_size_ota));

    //step3
    uint8_t begin_ota[64] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x33};
    write(fd, begin_ota, sizeof(begin_ota));

    transfer_file(fd, filename);
    
    //step4
    uint8_t crc_value_ota[64] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x22};
    memcpy(&crc_value_ota[6], pre_crc_num.data, 4);
    write(fd, crc_value_ota, sizeof(crc_value_ota));




    fclose(pd);
    return 0;
}


// 202604
// 0xd868f50b
#if 0
int main1()
{
    int total_size = 0;
    uint32_t pre_crc = 0;
    ifstream fin("../crab.signed.bin",ios::in | ios::binary);
    if(!fin.is_open()) exit(1);

    char buf[64];

    while(fin.readsome(buf, sizeof(buf)))
    {
        total_size += fin.gcount(); //read count
        print_array(buf, fin.gcount());
        cout<<endl;

        pre_crc = cy_ota_ble_crc32_update(pre_crc, (uint8_t *)buf, fin.gcount());
    }

    cout<<"total size = " << total_size << endl;
    printf("final crc = 0x%x\r\n", pre_crc);

    fin.close();

    cout << "hello world" << endl;

    return 0;
}
#endif
