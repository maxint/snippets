function videofromavi()

avifile = 'e:\object_tracking\video\Testcase\Distance Range\1-10.mpg';
readerobj = mmreader(avifile);
vidframes = read(readerobj, 1);
numframes = get(readerobj, 'numberOfFrames');
imshow(vidframes)