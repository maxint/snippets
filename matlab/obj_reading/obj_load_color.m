function clrList = obj_load_color(file_name, gIdx, ivtList)
%OBJ_LOAD_COLOR - Sample texture color of vertices from {dot} texture file
% Input:
%   file_name   obj file name
%   gIdx        group index of eyeL, eyeR and skin
%   ivtList     texcoord list of vertices
% Output:
%   Texture color list 
%  (Nv*3)
%   rgb         
%   rgb         
%   ...         
%   rgb       
%

%------------- BEGIN CODE --------------

% get color info of vertices from texture
groupList = char('eyeL_hi','eyeR_hi','skin_hi');
file_prefix = file_name(1:end-4);
Nv = max(size(ivtList));
clrList = zeros(Nv,3);
for i=1:size(gIdx,1)-1
    img = double(imread([file_prefix,'_',strtrim(groupList(i,:)),'.bmp'])) ./ 255.0;
    w = size(img,1);
    h = size(img,2);
    region = gIdx(i)+1:gIdx(i+1);
    for c=1:3
        clrList(region,c) = interp2(img(:,:,c), ivtList(region,1).*(w-1)+1,...
            ivtList(region,2).*(h-1)+1);
    end
end

%------------- END OF CODE --------------
