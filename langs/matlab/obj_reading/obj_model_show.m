function obj_model_show(pos3d, triList, clrMask)
%SHOW3DMODEL - Show a 3D model with gouraud shading or color mask
% Other m-files required: none
% Subfunctions: none
% MAT-files required: none
%
% See also: showres, showproj

% Author: maxint
% Email: lnychina {at} gmail {dot} com
% Dec 2010; Last revision: 10-Dec-2010

%------------- BEGIN CODE --------------

if (size(pos3d, 2) == 1)
    N = size(pos3d, 1) / 3;
    pos3d = reshape(pos3d, N, 3)';
end

if exist('clrMask', 'var')
    trisurf(triList, pos3d(1,:), pos3d(2,:), pos3d(3,:), clrMask);
else
    trisurf(triList, pos3d(1,:), pos3d(2,:), pos3d(3,:));
    light('Position',[150 300 150],'Style','infinite');
    lighting gouraud
    material dull 
    shading interp
    colormap(gray);
end

camproj('perspective')
axis equal
xlabel('x'); ylabel('y'); zlabel('z')

%------------- END OF CODE --------------