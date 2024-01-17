#define LOG_NDEBUG 0
#define LOG_TAG "ScreenCaptureTest"
#include "Log.h"
#include "ScreenCapture.h"
#define DUMP_BASEDIR "/data"
#define PORTTYPE_VALUE_VIDEO_ONLY_main_screen 0x11000000 //main screen
#define PORTTYPE_VALUE_VIDEO_OSD_main_screen 0x11000001 //main screen
#define PORTTYPE_VALUE_VIDEO_ONLY_second_screen 0x11000002 //second screen
#define PORTTYPE_VALUE_VIDEO_OSD_second_screen 0x11000003 //second screen


static int saveBufferAsFile(void *buffer, size_t size, char *file)
{
    int fd = open(file, O_CREAT|O_RDWR, 0666);
    int wrSize = 0;
    int ret = 0;

    if (fd < 0) {
        return fd;
    }

    do {
        int ret = write (fd, (char *)buffer + wrSize, size - wrSize);
        if (ret > 0) {
            wrSize += ret;
        } else {
            break;
        }
    } while (wrSize < size);
    close(fd);
    return ret;
}


static void checkAndSaveBufferToFile(const char *baseFile, char *filename, void *buffer, size_t size)
{
    if (0 == access(baseFile, F_OK)) {
        if (saveBufferAsFile(buffer, size, filename) >= 0) {
            printf("Save file ok: %s", filename);
        } else {
            printf("Save file fail: %s!", filename);
        }
    }
}

static void print_usage(void)
{
    printf ("Usage: screenrecorder\n");
    printf ("Options:\n\n");
    printf ("--help : display this help and exit\n");
    printf ("--width : you can set 3840 or 1920\n");
    printf ("--height : you can set 2160 or 1080\n");
    printf ("--framerate : you can set 30 or less than 30\n");
    printf ("--format :  you can set 1:nv21 2:nv12 \n");
    printf ("--layer : you can set 0:main screen video only 1:main screen video+osd 2:second screen video only 3:second screen video+osd\n");
    printf ("command sample: ./screenrecoder --width 1920 --height 1080 --framerate 30 --format 2 --layer 1\n");
    printf ("\n");
}

static ScreenCapture_Params* parse_command_line(int argc, char *argv[], ScreenCapture_Params *params)
{
    int i, j = 0;
    /* parse command line */
    for (i = 1; i < argc; i++) {
        if (strncmp (argv[i], "--", 2) == 0) {
            if (strcmp (argv[i] + 2, "help") == 0) {
                print_usage();
                return params;
            }
            else if (strcmp (argv[i] + 2, "width") == 0 && ++i < argc &&
                sscanf (argv[i], "%d", &params->width) == 1) {
                continue;
            }
            else if (strcmp (argv[i] + 2, "height") == 0 && ++i < argc &&
                sscanf (argv[i], "%dx%d", &params->height) == 1) {
                continue;
            }
            else if (strcmp (argv[i] + 2, "framerate") == 0 && ++i < argc &&
                sscanf (argv[i], "%dx%d", &params->framerate) == 1) {
                continue;
            }
            else if (strcmp (argv[i] + 2, "format") == 0 && ++i < argc &&
                sscanf (argv[i], "%dx%d", &params->data_type) == 1) {
                continue;
            }
            else if (strcmp (argv[i] + 2, "layer") == 0 && ++i < argc &&
                sscanf (argv[i], "%dx%d", &params->port_type) == 1) {
                switch (params->port_type) {
                case 0:
                    params->port_type = PORTTYPE_VALUE_VIDEO_ONLY_main_screen;
                    break;
                case 1:
                    params->port_type = PORTTYPE_VALUE_VIDEO_OSD_main_screen;
                    break;
                case 2:
                    params->port_type = PORTTYPE_VALUE_VIDEO_ONLY_second_screen;
                    break;
                case 3:
                    params->port_type = PORTTYPE_VALUE_VIDEO_OSD_second_screen;
                    break;
                default:
                    params->port_type = PORTTYPE_VALUE_VIDEO_OSD_main_screen;
                    break;
                }
                continue;
            }
        }
    }

    return params;
}

int main(int argc, char **argv) {
    int count = 0;
    ScreenCapture_Params params = {0};
    if (parse_command_line(argc, argv, &params) == nullptr) {
        printf ("parse_command_line fail\n");
        return 0;
    }
    int size = params.width * params.height * 3/2;
    auto capture = std::make_unique<ScreenCapture>();
    bool ret = capture->init(&params);
    if (!ret) {
        printf ("capture->init() fail\n");
        return 0;
    }
    ret = capture->start();
    if (!ret) {
        printf ("capture->start() fail\n");
        return 0;
    }
    char *buffer = (char *)malloc(size);
    while (1) {
        memset(buffer,0,size);
        ret = capture->readBuffer(buffer);
        if (!ret) {
            usleep(100);
            continue;
        }
        count++;
        char filename[64] = {0};
        const char *dump_path = DUMP_BASEDIR;
        snprintf(filename, 64, "%s/screen-capture-%d.yuv", dump_path, count);
        checkAndSaveBufferToFile(dump_path, filename, buffer, size);
        if (count >= 5)
            break;
    }
    free(buffer);
    capture->stop();
    return 0;
}

