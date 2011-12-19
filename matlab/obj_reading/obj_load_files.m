%% scan obj files
DIR_NAME = 'age';
TOTAL_PARAMS = 2;
MID_PARAM_IDX = 1; %int8(TOTAL_PARAMS / 2);

objFiles = dir([DIR_NAME '/*.obj']);
%% load triangle list
[triList, ivtList] = obj_load_trilist(fullfile(DIR_NAME, objFiles(1).name));
%% load vertices of obj files
Nobj = size(objFiles,1); % total num of directories
Nv = size(ivtList,1); % total num of vertices
shapeArray = zeros(Nv*3, Nobj);
for x=1:Nobj
    posList = obj_load_pos(fullfile(DIR_NAME, ...
        objFiles(x).name), Nv);
    shapeArray(:, x) = reshape(posList, Nv*3, 1);
end

%%
objList = {objFiles(:).name}'

%%
sK = csvread('sK.csv');
sM = csvread('sM.csv');
sL = csvread('sL.csv');

%%
%obj_model_show(sM, triList)
%%
sMs = reshape(sM, 3, Nv);
sM0 = csvread('sM0.csv');
rot_mat = rotationmat3D(8*pi/180.0,[1 0 0]);
sM1 = rot_mat * sM0;
vMax = max(sM1,[],2);
vMin = min(sM1,[],2);
vOff = (vMax+vMin)*0.5;
vScale = 2.0 / max(vMax-vMin);

sM3 =  pca_adjust_vertices(sM0, rot_mat, vOff, vScale);
s1 = sM3./sMs;
mean2(s1)
return

p = pca_get_params(shapeArray, rot_mat, vScale, sK);

sAge = sM - sK * p *2.0;
sAge = reshape(sAge, 3, Nv);
figure, obj_model_show(sAge, triList);

%%
Ns = max(size(sL));
sM = reshape(sM', Nv*3, 1);
sK = reshape(sK', Nv*3, Ns);

%%
alphaArr = sK \ (shapeArray - repmat(sM, 1, Nobj));

%%
% OK! - gender shape
rg = 1:TOTAL_PARAMS;
a = alphaArr(:,rg);
a = a - repmat(a(:,MID_PARAM_IDX),1,TOTAL_PARAMS);
d = a(:,2) - a(:,1);
aa = zeros(size(a));
for x=1:TOTAL_PARAMS
    aa(:,x) = a(:,1) + d*(x-1);
end
figure('Name', 'Is it a linear model?')
subplot(2,1,1),plot(a),title('Symmetirc view of original data')
subplot(2,1,2),plot(a),title('Synthetized data')

%%
rg = 1:TOTAL_PARAMS;

% first person
a = alphaArr(:,rg);
a = a - repmat(a(:,MID_PARAM_IDX),1,TOTAL_PARAMS);
d = a(:,2) - a(:,1);
aa = zeros(size(a));
for x=1:TOTAL_PARAMS
    aa(:,x) = a(:,1) + d*(x-1);
end
figure('Name', 'First Person')
subplot(2,1,1),plot(a),title('Symmetirc view of original data')
subplot(2,1,2),plot(a),title('Synthetized data')

% second person
rg = rg + TOTAL_PARAMS;
a = alphaArr(:,rg);
a = a - repmat(a(:,MID_PARAM_IDX),1,TOTAL_PARAMS);
d = a(:,2) - a(:,1);
aa = zeros(size(a));
for x=1:TOTAL_PARAMS
    aa(:,x) = a(:,1) + d*(x-1);
end
figure('Name', 'Second Person')
subplot(2,1,1),plot(a),title('Symmetirc view of original data')
subplot(2,1,2),plot(a),title('Synthetized data')
%% 
rg = 1:TOTAL_PARAMS;

% first person
a = alphaArr(:,rg);
a = a - repmat(a(:,MID_PARAM_IDX),1,TOTAL_PARAMS);
subplot(2,1,1),plot(a),title('First Person')

rg = rg + TOTAL_PARAMS;
a = alphaArr(:,rg);
a = a - repmat(a(:,MID_PARAM_IDX),1,TOTAL_PARAMS);
subplot(2,1,2),plot(a),title('Second Person')
%%
objlist = dir('*skin*.bmp');
objlist = {objlist(:).name}';
%%
for x=0:5
    img2 = imread(objlist{x*9+9});
    img1 = imread(objlist{x*9+1});
    d1 = img2 - img1;
    figure,imshow(d1)
end