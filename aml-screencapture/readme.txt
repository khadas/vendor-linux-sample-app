/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

screencapturetest user guide

API:

parse_command_line(argc, argv, &params)
/*
&params: you need set some params in the command_line, like width & height & framerate & data_type & port_type
 */

capture->init(&params)
/*
The parse_command_line function sets the input params down via capture->init()
 */

capture->start()
/*
This will call screencapture.cpp interface, you don't need to worry about how to implement,
the specific function will open the device node, pass parameters, register callback, ioctl, etc.
 */

capture->readBuffer(buffer)
/*
You can access the contents of the buf through this interface
 */

checkAndSaveBufferToFile(dump_path, filename, buffer, size);
/*
In this interface you can get the buf to write to the file, test is to write to the /data directory
Of course, the specific size needs you to apply for allocation directly
 */

test result;
/*
After running the tests, you should see a few frames of yuv data in the /data directory, which you can pull out and view with tools
 */

usage examples:
./screencapture --width 1920 --height 1080 --framerate 30 --format 2 --layer 1
