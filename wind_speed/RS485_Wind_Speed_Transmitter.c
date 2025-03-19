#include "RS485_Wind_Speed_Transmitter.h"

int Sfd;

char InitSensor(char *dev) {
  struct termios tio;
  // int tty_fd;

  memset(&tio, 0, sizeof(tio));
  tio.c_iflag = 0;
  tio.c_oflag = 0;
  tio.c_cflag = CS8 | CREAD | CLOCAL;
  tio.c_lflag = 0;
  tio.c_cc[VMIN] = 1;
  tio.c_cc[VTIME] = 5;

  Sfd = open(dev, O_RDWR | O_NONBLOCK);
  if (Sfd < 0) {
    perror("open");
    return 0;
  }

  cfsetospeed(&tio, B9600);
  cfsetispeed(&tio, B9600);

  tcsetattr(Sfd, TCSANOW, &tio);
  return 1;
}

void addedCRC(unsigned char *buf, int len) {
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (unsigned int)buf[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  buf[len] = crc % 0x100;
  buf[len + 1] = crc / 0x100;
}

unsigned char ModifyAddress(unsigned char Address1, unsigned char Address2) {
  unsigned char ModifyAddressCOM[11] = { 0x00, 0x10, 0x10, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00 };
  char ret = 0;
  struct timeval curr, curr1, curr2;
  long mtime, seconds, useconds;
  gettimeofday(&curr, NULL);
  curr1 = curr;
  char ch = 0;
  ModifyAddressCOM[0] = Address1;
  ModifyAddressCOM[8] = Address2;
  addedCRC(ModifyAddressCOM, 9);

  // for (int i = 0; i < 11; i++)
  // {
  //   printf("%02X ", ModifyAddressCOM[i]);
  // }
  // printf("\n");

  write(Sfd, ModifyAddressCOM, 11);
  while (!ret) {
    gettimeofday(&curr2, NULL);
    seconds = curr2.tv_sec - curr.tv_sec;
    useconds = curr2.tv_usec - curr.tv_usec;
    mtime = ((seconds)*1000 + useconds / 1000.0) + 0.5;

    if (mtime > 1000) {
      break;
    }

    gettimeofday(&curr2, NULL);
    seconds = curr2.tv_sec - curr1.tv_sec;
    useconds = curr2.tv_usec - curr1.tv_usec;
    mtime = ((seconds)*1000 + useconds / 1000.0) + 0.5;

    if (mtime > 100) {
      write(Sfd, ModifyAddressCOM, 11);
      gettimeofday(&curr1, NULL);
    }

    delayms(20);
    if (read(Sfd, &ch, 1) == 1) {
      if (ch == Address1) {
        if (read(Sfd, &ch, 1) == 1) {
          if (ch == 0x10) {
            if (read(Sfd, &ch, 1) == 1) {
              if (ch == 0x10) {
                if (read(Sfd, &ch, 1) == 1) {
                  if (ch == 0x00) {
                    if (read(Sfd, &ch, 1) == 1) {
                      if (ch == 0x00) {
                        if (read(Sfd, &ch, 1) == 1) {
                          if (ch == 0x01) {
                            ret = 1;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return ret;
}

float readWindSpeed(unsigned char Address) {
  unsigned char Data[7] = { 0 };
  unsigned char COM[8] = { 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00 };
  char ret = 0;
  float WindSpeed = 0;
  struct timeval curr, curr1, curr2;
  long mtime, seconds, useconds;
  gettimeofday(&curr, NULL);
  curr1 = curr;

  char ch = 0;
  COM[0] = Address;
  addedCRC(COM, 6);

  // for (int i = 0; i < 8; i++)
  // {
  //   printf("%02X ", COM[i]);
  // }
  // printf("\n");

  write(Sfd, COM, 8);
  while (!ret) {
    gettimeofday(&curr2, NULL);
    seconds = curr2.tv_sec - curr.tv_sec;
    useconds = curr2.tv_usec - curr.tv_usec;
    mtime = ((seconds)*1000 + useconds / 1000.0) + 0.5;

    if (mtime > 1000) {
      WindSpeed = -1;
      break;
    }

    gettimeofday(&curr2, NULL);
    seconds = curr2.tv_sec - curr1.tv_sec;
    useconds = curr2.tv_usec - curr1.tv_usec;
    mtime = ((seconds)*1000 + useconds / 1000.0) + 0.5;

    if (mtime > 100) {
      write(Sfd, COM, 8);
      gettimeofday(&curr1, NULL);
    }

    delayms(10);
    if (read(Sfd, &ch, 1) == 1) {
      if (ch == Address) {
        Data[0] = ch;
        if (read(Sfd, &ch, 1) == 1) {
          if (ch == 0x03) {
            Data[1] = ch;
            if (read(Sfd, &ch, 1) == 1) {
              if (ch == 0x02) {
                Data[2] = ch;
                if (read(Sfd, &Data[3], 4) == 4) {
                  if (CRC16_2(Data, 5) == (Data[5] * 256 + Data[6])) {
                    // for (int i = 0; i < 7; i++)
                    // {
                    //   printf("%02X ", Data[i]);
                    // }
                    // printf("\n");

                    ret = 1;
                    WindSpeed = (Data[3] * 256 + Data[4]) / 10.00;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return WindSpeed;
}

unsigned int CRC16_2(unsigned char *buf, int len) {
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (unsigned int)buf[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }

  crc = ((crc & 0x00ff) << 8) | ((crc & 0xff00) >> 8);
  return crc;
}

void delayms(int data) {
  usleep(data * 1000);
}