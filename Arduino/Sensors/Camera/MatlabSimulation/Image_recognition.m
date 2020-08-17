%% Global declarations
m = 240;    % Image height
n = 320;    % Image width
num_m = 27; % Number height
num_n = 18; % Number width

%% Sample numbers

% Creation of variables
nums = zeros(num_m, num_n, 10, 'uint8');
nums_x = zeros(10, num_n);
nums_y = zeros(num_m, 10);

% Colour treatment and image cut
for i = 0:9
    aux = processImage(m,n,imread(strcat('C:\Users\Usuario\Desktop\Estudios\Universidad\Semestre 8 - Erasmus\TFG\Sensores\ArduCAM\Reconocimiento\Simulacion\N', int2str(i), '.png')));
    aux = deleteTopMargin(aux);
    [~, aux] = getNextNumber(aux, num_m, num_n);
    nums(:,:,i+1) = aux;
    nums_x(i+1,:) = sum(nums(:,:,i+1), 1);
    nums_y(:,i+1) = sum(nums(:,:,i+1), 2);
end

%% Read and process the image

% Colour treatment
image = processImage(m,n,imread('C:\Users\Usuario\Desktop\Estudios\Universidad\Semestre 8 - Erasmus\TFG\Sensores\ArduCAM\Reconocimiento\Simulacion\I1.png'));
%%%imshow(image),title(sprintf('Imagen escala grises'));

% Vertical size cut
imageP = deleteTopMargin(image);
imageP = deleteButtomMargin(imageP);

% Delete of "km" word
imageP = deleteRightMargin(imageP);
imageP = deleteRightObject(imageP);
imageP = deleteRightMargin(imageP);
imageP = deleteRightObject(imageP);

% Separation and identification of each number
kms = zeros(num_m, num_n, 6);
kms_proc = zeros(1,6);
for i=1:6
    [imageP, kms(:,:,6-i+1)] = getNextNumber(imageP, num_m, num_n);
    kms_proc(6-i+1) = processNumber(kms(:,:,6-i+1), nums_x, nums_y, num_m, num_n);
end

%% Final result
odometer = 0;
for i=1:6
    odometer = odometer + kms_proc(6-i+1)*10^(i-1);
end
odometer