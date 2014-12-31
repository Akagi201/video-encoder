/*
 * @file x265_encoder.c
 * @author Akagi201
 * @date 2014/12/31
 */

#include <stdio.h>
#include <stdlib.h>
#include <x265.h>

#include "lwlog/lwlog.h"

int main(int argc, char **argv) {
    int i = 0;
    int j = 0;
    FILE *fp_src = NULL;
    FILE *fp_dst = NULL;
    int y_size = 0;
    int buff_size = 0;
    char *buff = NULL;
    int ret = 0;
    x265_nal *p_nals = NULL;
    uint32_t i_nal = 0;

    x265_param *p_param = NULL;
    x265_encoder *p_handle = NULL;
    x265_picture *p_pic_in = NULL;

    //Encode 50 frame
    //if set 0, encode all frame
    int frame_num = 50;
    int csp = X265_CSP_I420;
    int width = 640;
    int height = 360;

    fp_src = fopen("../cuc_ieschool_640x360_yuv420p.yuv", "rb");
    //fp_src=fopen("../cuc_ieschool_640x360_yuv444p.yuv","rb");

    fp_dst = fopen("cuc_ieschool.h265", "wb");
    //Check
    if ((NULL == fp_src) || (NULL == fp_dst)) {
        lwlog_err("Open files err");
        return -1;
    }

    p_param = x265_param_alloc();
    x265_param_default(p_param);
    p_param->bRepeatHeaders = 1;//write sps,pps before keyframe
    p_param->internalCsp = csp;
    p_param->sourceWidth = width;
    p_param->sourceHeight = height;
    p_param->fpsNum = 25;
    p_param->fpsDenom = 1;
    //Init
    p_handle = x265_encoder_open(p_param);
    if (NULL == p_handle) {
        lwlog_err("x265_encoder_open err");
        return -1;
    }

    y_size = p_param->sourceWidth * p_param->sourceHeight;

    p_pic_in = x265_picture_alloc();
    x265_picture_init(p_param, p_pic_in);
    switch (csp) {
        case X265_CSP_I444: {
            buff = (char *) malloc(y_size * 3);
            p_pic_in->planes[0] = buff;
            p_pic_in->planes[1] = buff + y_size;
            p_pic_in->planes[2] = buff + y_size * 2;
            p_pic_in->stride[0] = width;
            p_pic_in->stride[1] = width;
            p_pic_in->stride[2] = width;
            break;
        }
        case X265_CSP_I420: {
            buff = (char *) malloc(y_size * 3 / 2);
            p_pic_in->planes[0] = buff;
            p_pic_in->planes[1] = buff + y_size;
            p_pic_in->planes[2] = buff + y_size * 5 / 4;
            p_pic_in->stride[0] = width;
            p_pic_in->stride[1] = width / 2;
            p_pic_in->stride[2] = width / 2;
            break;
        }
        default: {
            printf("Colorspace Not Support.\n");
            return -1;
        }
    }

    //detect frame number
    if (0 == frame_num) {
        fseek(fp_src, 0, SEEK_END);
        switch (csp) {
            case X265_CSP_I444:
                frame_num = ftell(fp_src) / (y_size * 3);
                break;
            case X265_CSP_I420:
                frame_num = ftell(fp_src) / (y_size * 3 / 2);
                break;
            default:
                printf("Colorspace Not Support.\n");
                return -1;
        }
        fseek(fp_src, 0, SEEK_SET);
    }

    //Loop to Encode
    for (i = 0; i < frame_num; ++i) {
        switch (csp) {
            case X265_CSP_I444: {
                fread(p_pic_in->planes[0], 1, y_size, fp_src);        //Y
                fread(p_pic_in->planes[1], 1, y_size, fp_src);        //U
                fread(p_pic_in->planes[2], 1, y_size, fp_src);        //V
                break;
            }
            case X265_CSP_I420: {
                fread(p_pic_in->planes[0], 1, y_size, fp_src);        //Y
                fread(p_pic_in->planes[1], 1, y_size / 4, fp_src);    //U
                fread(p_pic_in->planes[2], 1, y_size / 4, fp_src);    //V
                break;
            }
            default: {
                printf("Colorspace Not Support.\n");
                return -1;
            }
        }

        ret = x265_encoder_encode(p_handle, &p_nals, &i_nal, p_pic_in, NULL);

        lwlog_info("Succeed encode %5d frames", i);

        for (j = 0; j < i_nal; ++j) {
            fwrite(p_nals[j].payload, 1, p_nals[j].sizeBytes, fp_dst);
        }
    }
    //Flush Decoder
    while (1) {
        ret = x265_encoder_encode(p_handle, &p_nals, &i_nal, NULL, NULL);
        if (0 == ret) {
            break;
        }
        lwlog_info("Flush 1 frame.");

        for (j = 0; j < i_nal; ++j) {
            fwrite(p_nals[j].payload, 1, p_nals[j].sizeBytes, fp_dst);
        }
    }

    x265_encoder_close(p_handle);
    x265_picture_free(p_pic_in);
    x265_param_free(p_param);
    free(buff);
    fclose(fp_src);
    fclose(fp_dst);

    return 0;
}