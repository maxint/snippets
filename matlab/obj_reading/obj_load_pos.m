function posList = obj_load_pos(file_name, num)
%OBJ_LOAD_POS - Load position of vertices from {dot} obj file
% Input:
%   file_name   obj file name
%   num         number of vertices
% Output:
%   Vertex position list 
% posList (Nv*3)
%   xyz         
%   xyz         
%   ...         
%   xyz       
%
% Other m-files required: none
% Subfunctions: none
% MAT-files required: none
%
% See also: OTHER_FUNCTION_NAME1,  OTHER_FUNCTION_NAME2

%------------- BEGIN CODE --------------

fid = fopen(file_name, 'r');
format_v3 = 'v %f %f %f';
posList = zeros(num,3);
verNum = 0;
while ~feof(fid) && verNum<num
    tline = fgetl(fid);
    if ~isempty(tline) && strcmp(tline(1:2),'v ')
        verLine = sscanf(tline, format_v3);
        verNum = verNum+1;
        posList(verNum,:) = verLine';
    end
end

%------------- END OF CODE --------------
