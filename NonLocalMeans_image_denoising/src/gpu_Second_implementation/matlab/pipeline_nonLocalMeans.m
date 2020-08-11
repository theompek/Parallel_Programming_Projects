function [ ] = gpu_Pipeline_nonLocalMean(patchSize,sigma,imageName)
    
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

   
  
  k = parallel.gpu.CUDAKernel( '../cuda/gpu_kernel.ptx', ...
                               '../cuda/gpu_kernel.cu');
  fprintf('...begin %s...\n',mfilename); 
  
  numberOfBlocks  = ceil( [m n] ./ threadsPerBlock );
  
  %������� �� ��� 2 ���������� ��� block ������������ ��� ����� ��� 
  %��� ������������� ���� ��������� numberOfBlocks
  DimZ=numberOfBlocks(1)*numberOfBlocks(2);
  numberOfBlocks=horzcat(numberOfBlocks,DimZ);

  %���������� ��� blocks
  k.ThreadBlockSize = threadsPerBlock;
  %���������� ��� Grid
  k.GridSize        = numberOfBlocks;
  
  
  %������������ ������� ���� Gpu
  OutImage = zeros([m n], 'gpuArray');
  Zi= zeros([m n], 'gpuArray');
  NoisingImage = gpuArray(J);%��������� ��� ������� ���� gpu
 
  %��������� ��������� �� ���� float
  NoisingImage=single(NoisingImage); 
  OutImage=single(OutImage);
  Zi=single(Zi); 

  HalfPacthWidth=(patchSize-1)/2;
  
  fprintf('The algorithm Gpu-Non-Local-Mean begins running...\n' );
  tic;
  %������ ������������ ����������� ��� ��������� �� ������������ ����� ��� ����� ��� Zi ��� ���� pixel
  [OutImage,Zi]=feval(k,NoisingImage,OutImage,Zi,HalfPacthWidth,imageSizePixel,sigma,1);
  
  %����������� �� ������������ �� Zi
  wait(gpuDevice);
  fprintf('To Zi ypologistike' );
  
  %������������ ��� ���� ����� ��� pixel
  OutImage=gather( feval(k,NoisingImage,OutImage,Zi,HalfPacthWidth,imageSizePixel,sigma,0));
  fprintf('The time of execution of the program---> ' );
  toc 
 
  %% (END)

  fprintf('\n...end %s...\n',mfilename);
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

