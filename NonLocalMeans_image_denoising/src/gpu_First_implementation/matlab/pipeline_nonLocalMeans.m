function [ ] = gpu_Pipeline_nonLocalMean(patchSize,sigma,imageName )
  
%patchSize:������� ��� patch,������� ����� 3,5,7,9,11,...���
%sigma:���� ��� ���������� �
%imageName:�� ����� ��� ������� �� string,������ ��� ������ 'house',
          %�� ������� ������ �� ���������� ��� ������ data

		  
 %% PARAMETERS
  
  % input image
  strImgVar=imageName;
  path='../data/';
  pathImg=strcat(path,imageName,'.mat');
  

  %�� path ��� ��� ���������� ��� �������
  fname = '../images/';
  
  % noise
  noiseParams = {'gaussian', ...
                 0,...
                 0.001};
  
  
  %% USEFUL FUNCTIONS

  % image normalizer
  normImg = @(I) (I - min(I(:))) ./ max(I(:) - min(I(:)));
  
  %% (BEGIN)

  fprintf('...begin %s...\n',mfilename);  
  
  %% INPUT DATA
  
  fprintf('...loading input data...\n')
  
  ioImg = matfile( pathImg );
  I     = ioImg.(strImgVar);
  
  %% PREPROCESS
  
  fprintf(' - normalizing image...\n')
  I = normImg( I );
  
  figure('Name','Original Image');
  imagesc(I); axis image;
  colormap gray;
   
   saveas(gcf,fullfile(fname,'Original Image')); %���������� ��� ������� ������� 
  %��������� �� double 
  I=double(I);
  
  %% NOISE
  
  fprintf(' - applying noise...\n')
  J = imnoise( I, noiseParams{:} );
  figure('Name','Noisy-Input Image');
  imagesc(J); axis image;
  colormap gray;
   saveas(gcf,fullfile(fname,'Noisy_Image.fig'));%���������� ��� ������� �� ������ 
  
  %% NON LOCAL MEANS GPU IMPLEMENTATION
 %====================================
 %% PARAMETERS
  blockSize=16;
  imageSizePixel= length(I); %���������� �������
  threadsPerBlock = [blockSize blockSize];
  m = imageSizePixel;
  n = imageSizePixel;

   
  
  k = parallel.gpu.CUDAKernel( '../cuda/GPUnonLocalMean.ptx', ...
                               '../cuda/GPUnonLocalMean.cu');
  fprintf('...begin %s...\n',mfilename); 
  
  numberOfBlocks  = ceil( [m n] ./ threadsPerBlock );
  %���������� ��� blocks
  k.ThreadBlockSize = threadsPerBlock;
  %���������� ��� Grid
  k.GridSize        = numberOfBlocks;
    
  %������������ ������� ���� Gpu
  OutImage = zeros([m n], 'gpuArray');
  HalfPacthWidth=(patchSize-1)/2;
  NoisingImage = gpuArray(J);%��������� ��� ������� ���� gpu
 
  %��������� ��������� �� ���� float
  NoisingImage=single(NoisingImage); 
  OutImage=single(OutImage);
  
  fprintf('The algorithm Gpu-Non-Local-Mean begins running...\n' );
  tic;
  OutImage=gather( feval(k,NoisingImage,OutImage,HalfPacthWidth,imageSizePixel,sigma));
  fprintf('The time of execution of the program---> ' );
  toc 
    
  %% (END)

  fprintf('...end %s...\n',mfilename);
 %====================================
 %% VISUALIZE RESULT
  figure('Name', 'Filtered image');
  imagesc(OutImage); axis image;
  colormap gray;
  saveas(gcf,fullfile(fname,'Filtered_Image.fig')); %���������� ��� ������������������ ������� 
  
  figure('Name', 'Residual');
  imagesc(OutImage-J); axis image;
  colormap gray;
  saveas(gcf,fullfile(fname,'Residual.fig'));%���������� ��� ������� ��� ��������
  %% (END)

  fprintf('...end %s...\n',mfilename);


end

