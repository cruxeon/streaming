#include <stdio.h>
#include "h264_stream.h"


int main() {

    unsigned char msg[5] = "hello";

    //Write SEI NAL unit
    h264_stream_t* h = h264_new();

    h->nal->nal_ref_idc = NAL_REF_IDC_PRIORITY_HIGHEST;
    h->nal->nal_unit_type = NAL_UNIT_TYPE_SEI;

    h->sei->payloadType = SEI_TYPE_USER_DATA_UNREGISTERED;
    h->sei->payloadSize = 5;
    h->sei->payload = msg;

    unsigned int size = 1000;
    unsigned char buf[size];
    int len = write_nal_unit(h, buf, size);

    printf("size used %d\n", len);

    return 0;
}
