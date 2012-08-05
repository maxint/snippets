function [triList, ivtList, gIdx] = obj_load_trilist(filename)
%OBJ_LOAD_TRILIST - Load triangle list from {dot} obj file
% Input:
%   filename    obj file name
% Output:
%   triList     triangle list, Nv x 3 (index)
%   ivtList     texcoord list, Nv x 2 (value)
%   gIdx        increasing group indices, 1024, 2048(1024+1024)
%
% Other m-files required: none
% Subfunctions: none
% MAT-files required: none
%
% See also: OTHER_FUNCTION_NAME1,  OTHER_FUNCTION_NAME2

%------------- BEGIN CODE --------------

MAX_GROUP_NUM = 4;

Ntri = 0;   % total number of triangles
Nvt = 0;    % total number of texcoords
Nv = 0;     % total number of vertices
gNum = 0;   % total number of groups
gIdx = zeros(MAX_GROUP_NUM,1);  % indices of groups
fid = fopen(filename, 'r');

% g eyeR_hi
% s 1
% v 41.5325 19.9112 48.7261
% ...
% vt 0.140006 0.281843
% ...
% usemtl Texture0
% f 48/48 2/1 46/46 45/45

% First pass, count the number of triangles, vertices and texcoords
while ~feof(fid)
    tline = fgetl(fid);
    if ~isempty(tline)
        switch tline(1:2)
            case 'g '
                gNum = gNum+1;
                gIdx(gNum) = Nv;
                if length(tline)==6 && strcmp(tline(3:6),'sock'), break, end
            case 'vt'
                Nvt = Nvt+1;
            case 'v '
                Nv = Nv+1;
            case 'f '
                str_type = length(findstr(strtrim(tline), ' '));
                if str_type<=4
                    Ntri = Ntri+str_type-2;
                else
                    disp 'Unexpected input!'
                end
        end
    end
end

% Second pass, read triangle list and texcoords
format_vt2 = 'vt %f %f';
format_3 = 'f %d/%d %d/%d %d/%d'; % v/vt v/vt v/vt
format_4 = 'f %d/%d %d/%d %d/%d %d/%d'; % v/vt v/vt v/vt v/vt
triNum = 0; % current number of triangles
vtNum = 0;  % current number of texcoords
triList = zeros(Ntri,6); % v/vt v/vt v/vt
vtList = zeros(Nvt,2);
vtvIdx = zeros(Nv,1); % texcoord index of vertices
fseek(fid, 0, 'bof');
while ~feof(fid)
    tline = fgetl(fid);
    if ~isempty(tline)        
        switch tline(1:2)
            case 'g '
                if length(tline)==6 && strcmp(tline(3:6),'sock'), break, end
            case 'vt'
                vtLine = sscanf(tline, format_vt2);
                vtNum = vtNum+1;
                vtList(vtNum,:) = vtLine';
            case 'f '
                str_type = length(findstr(strtrim(tline), ' '));
                if str_type==3
                    faceList = sscanf(tline, format_3, 1);
                    triNum = triNum+1;
                    triList(triNum,:) = faceList';
                elseif str_type==4
                    faceList = sscanf(tline, format_4);
                    triNum = triNum+1;
                    triList(triNum,:) = faceList(1:6)';
                    triNum = triNum+1;
                    triList(triNum,:) = [faceList(5:8); faceList(1:2)]';
                end
        end
    end
end
%assert (Ntri==triNum && Nvt==vtNum);
rgV = 1:2:5; % [1 3 5]
% NOTE: 假设纹理的序号和顶点的序号是一一对应的，但是不一定
for i=1:Ntri
   vtvIdx(triList(i,rgV)) = triList(i,rgV+1);
end
ivtList = zeros(Nv,2);
for i=1:Nv
    ivtList(i,:) = vtList(vtvIdx(i),:);
end
ivtList = max(min(ivtList,1),0);
% NOTE: invert y coordinate
ivtList(:,2) = 1.0 - ivtList(:,2);
triList = triList(:,1:2:5); % v/vt v/vt v/vt => v v v (index)

%------------- END OF CODE --------------
