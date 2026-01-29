#ifndef _ROS_custom_msgs_Waypoint_h
#define _ROS_custom_msgs_Waypoint_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace custom_msgs
{

  class Waypoint : public ros::Msg
  {
    public:
      typedef float _lat_type;
      _lat_type lat;
      typedef float _lng_type;
      _lng_type lng;

    Waypoint():
      lat(0),
      lng(0)
    {
    }

    virtual int serialize(unsigned char *outbuffer) const override
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_lat;
      u_lat.real = this->lat;
      *(outbuffer + offset + 0) = (u_lat.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_lat.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_lat.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_lat.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->lat);
      union {
        float real;
        uint32_t base;
      } u_lng;
      u_lng.real = this->lng;
      *(outbuffer + offset + 0) = (u_lng.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_lng.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_lng.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_lng.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->lng);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer) override
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_lat;
      u_lat.base = 0;
      u_lat.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_lat.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_lat.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_lat.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->lat = u_lat.real;
      offset += sizeof(this->lat);
      union {
        float real;
        uint32_t base;
      } u_lng;
      u_lng.base = 0;
      u_lng.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_lng.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_lng.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_lng.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->lng = u_lng.real;
      offset += sizeof(this->lng);
     return offset;
    }

    virtual const char * getType() override { return "custom_msgs/Waypoint"; };
    virtual const char * getMD5() override { return "321bf96dbd44cdb44c4685a0ded4bea4"; };

  };

}
#endif
