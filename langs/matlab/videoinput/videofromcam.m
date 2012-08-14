function [ output_args ] = videofromcam( input_args )
%VIDEOFROMCAM Summary of this function goes here
%   Detailed explanation goes here

%-- code from TLD
% Initialization of camera
% Try to run independently in Matlab (requires Image Acquisition toolbox)
% if it does not work, modify the first two lines

%source.vid = videoinput('winvideo', 1);
%source.vid = videoinput('winvideo', 1,'RGB24_320x240');
%source.vid = videoinput('winvideo', 1,'YUY2_320x240');
source.vid = videoinput('winvideo', 1, 'YUY2_640x480');

%set(source.vid,'ReturnedColorSpace','grayscale');
% vidRes = get(source.vid, 'VideoResolution');
% nBands = get(source.vid, 'NumberOfBands');
%hImage = image( zeros(vidRes(2), vidRes(1), nBands) );

% open a figure that shows the raw video stream
%preview(source.vid, hImage);
preview(source.vid)
% makes the raw stream invisible
%set(1,'visible','off');

end

