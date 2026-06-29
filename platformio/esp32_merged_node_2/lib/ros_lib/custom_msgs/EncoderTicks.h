#ifndef _ROS_custom_msgs_EncoderTicks_h
#define _ROS_custom_msgs_EncoderTicks_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace custom_msgs
{

  class EncoderTicks : public ros::Msg
  {
    public:
      typedef std_msgs::Header _header_type;
      _header_type header;
      typedef int32_t _ticks_type;
      _ticks_type ticks;
      typedef int32_t _delta_ticks_type;
      _delta_ticks_type delta_ticks;

    EncoderTicks():
      header(),
      ticks(0),
      delta_ticks(0)
    {
    }

    virtual int serialize(unsigned char *outbuffer) const override
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      union {
        int32_t real;
        uint32_t base;
      } u_ticks;
      u_ticks.real = this->ticks;
      *(outbuffer + offset + 0) = (u_ticks.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_ticks.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_ticks.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_ticks.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->ticks);
      union {
        int32_t real;
        uint32_t base;
      } u_delta_ticks;
      u_delta_ticks.real = this->delta_ticks;
      *(outbuffer + offset + 0) = (u_delta_ticks.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_delta_ticks.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_delta_ticks.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_delta_ticks.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->delta_ticks);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer) override
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      union {
        int32_t real;
        uint32_t base;
      } u_ticks;
      u_ticks.base = 0;
      u_ticks.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_ticks.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_ticks.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_ticks.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->ticks = u_ticks.real;
      offset += sizeof(this->ticks);
      union {
        int32_t real;
        uint32_t base;
      } u_delta_ticks;
      u_delta_ticks.base = 0;
      u_delta_ticks.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_delta_ticks.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_delta_ticks.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_delta_ticks.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->delta_ticks = u_delta_ticks.real;
      offset += sizeof(this->delta_ticks);
     return offset;
    }

    virtual const char * getType() override { return "custom_msgs/EncoderTicks"; };
    virtual const char * getMD5() override { return "b02ec1224acd3c659bd318623a2c19a2"; };

  };

}
#endif
