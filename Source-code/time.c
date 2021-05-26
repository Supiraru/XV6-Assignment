// uptime. How long has xv6 been up
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int start_time = uptime();

  if (argc == 1){
      int rem = 0;
      int current_time = uptime() - start_time;
      if(current_time > 1000){
        rem = current_time % 1000;
        current_time /= 1000;
    }
      if(rem != 0){
        printf(1, "(null) ran in %d.%d seconds\n", current_time, rem);
    } else{
        printf(1, "(null) ran in 0.%d seconds\n", current_time);
    }
        //if doesnt have any arg
    } else {
        if (fork() == 0){
            exec(argv[1], &argv[1]);
        }else{
            wait();
             int rem = 0;
             int current_time = uptime() - start_time;
            
            if(current_time > 1000){
            rem = current_time % 1000;
            current_time /= 1000;
            }
            if(rem == 0){
                printf(1, "%s ran in 0.%d seconds\n", argv[1], current_time);
            } else{
                printf(1, "%s ran in %d.%d seconds\n", argv[1], current_time, rem);
            }
        }
    }

  exit();
}
