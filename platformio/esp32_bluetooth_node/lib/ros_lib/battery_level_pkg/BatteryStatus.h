#ifndef _ROS_SERVICE_BatteryStatus_h
#define _ROS_SERVICE_BatteryStatus_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace battery_level_pkg
{

static const char BATTERYSTATUS[] = "battery_level_pkg/BatteryStatus";

  class BatteryStatusRequest : public ros::Msg
  {
    public:

    BatteryStatusRequest()
    {
    }

    virtual int serialize(unsigned char *outbuffer) const override
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer) override
    {
      int offset = 0;
     return offset;
    }

    virtual const char * getType() override { return BATTERYSTATUS; };
    virtual const char * getMD5() override { return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class BatteryStatusResponse : public ros::Msg
  {
    public:
      typedef float _battery_level_type;
      _battery_level_type battery_level;

    BatteryStatusResponse():
      battery_level(0)
    {
    }

    virtual int serialize(unsigned char *outbuffer) const override
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_battery_level;
      u_battery_level.real = this->battery_level;
      *(outbuffer + offset + 0) = (u_battery_level.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_battery_level.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_battery_level.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_battery_level.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->battery_level);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer) override
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_battery_level;
      u_battery_level.base = 0;
      u_battery_level.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_battery_level.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_battery_level.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_battery_level.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->battery_level = u_battery_level.real;
      offset += sizeof(this->battery_level);
     return offset;
    }

    virtual const char * getType() override { return BATTERYSTATUS; };
    virtual const char * getMD5() override { return "0e41eca1fafc4a04b193de5c7d2698c1"; };

  };

  class BatteryStatus {
    public:
    typedef BatteryStatusRequest Request;
    typedef BatteryStatusResponse Response;
  };

}
#endif
