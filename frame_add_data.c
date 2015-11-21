#include <stdio.h>
#include "h264_stream.h"


int main() {
    
    h264_stream_t* h = h264_new();

    unsigned char msg[5] = "hello";
    
    
    //Write SEI NAL unit

    h->nal->nal_ref_idc = NAL_REF_IDC_PRIORITY_HIGHEST;
    h->nal->nal_unit_type = NAL_UNIT_TYPE_SEI;
    
    h->num_seis = 1;
    h->seis = (sei_t**)realloc(h->seis, h->num_seis * sizeof(sei_t*));
    
    unsigned int i;
    for (i = 0; i < h->num_seis; i++) {
        
        sei_t* s = sei_new();
        s->payloadType = SEI_TYPE_USER_DATA_UNREGISTERED;
        s->payloadSize = 5;
        s->payload = msg;
        
        h->seis[i] = s;
    }


    /*
    h->nal->nal_ref_idc = 0x03;
    h->nal->nal_unit_type = NAL_UNIT_TYPE_PPS;

    h->sps->profile_idc = 0x42;
    h->sps->level_idc = 0x33;
    h->sps->log2_max_frame_num_minus4 = 0x05;
    h->sps->log2_max_pic_order_cnt_lsb_minus4 = 0x06;
    h->sps->num_ref_frames = 0x01;
    h->sps->pic_width_in_mbs_minus1 = 0x2c;
    h->sps->pic_height_in_map_units_minus1 = 0x1d;
    h->sps->frame_mbs_only_flag = 0x01;*/

    unsigned int size = 1000;
    unsigned char buf[size];
    int len = write_nal_unit(h, buf, size);

    printf("size used %d\n", len);

    return 0;
}
