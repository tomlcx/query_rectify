/**
 * Copyright 2011 DangDang Inc. �����з���
 * All rights reserved.
 *
 * �ļ�����: TimeUtil.h
 * ժ    Ҫ: ����һ�γ������ִ��ʱ��ͳ��
 *
 * ��ǰ�汾: 1.0
 * ��    ��: ���賬
 * �������: 2011-06-20
 */

#ifndef _PRODUCT_INFO_UPDATER_TIME_UTIL_H_
#define _PRODUCT_INFO_UPDATER_TIME_UTIL_H_

#include <sys/time.h>

class TimeUtil
{
public:
    /**
     * ��ʼ����ǰ����������ʱ��
     */
    TimeUtil()
    {
        gettimeofday(&start_time, NULL);
    }
    
    /**
     * ��������: ��ȡ�ӵ�ǰ�����������˷������þ����ĺ�����
     * �������: void
     * ����ֵ  : �����ĺ�����
     *
     * ����:
     * {
     *   TimeUtil tu;
     *   // some code
     *   long passTime = tu.getPassedTime();
     * }
     */
    int getPassedTime()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        int r = (now.tv_sec - start_time.tv_sec) * 1000;
        r += (now.tv_usec - start_time.tv_usec) / 1000;
        return r;
    }
    
private:
    struct timeval start_time;
};

#endif // _PRODUCT_INFO_UPDATER_TIME_UTIL_H_

