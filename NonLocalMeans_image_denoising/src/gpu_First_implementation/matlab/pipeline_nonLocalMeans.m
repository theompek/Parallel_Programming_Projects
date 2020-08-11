function [ ] = gpu_Pipeline_nonLocalMean(patchSize,sigma,imageName )
  
%patchSize:Μέγεθος του patch,παίρνει τιμές 3,5,7,9,11,...κλπ
%sigma:Τιμή της μεταβλητής σ
%imageName:Το ονομα της εικόνας ως string,δηλαδή της μορφής 'house',
          %οι εικόνες πρέπει να βρίσκονται στο φάκελο data

		  
 %% PARAMETERS
  
  % input image
  strImgVar=imageName;
  path='../data/';
  pathImg=strcat(path,imageName,'.mat');
  

  %Το path για την αποθήκευση των εικόνων
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
   
   saveas(gcf,fullfile(fname,'Original Image')); %Αποθήκευση της αρχικής εικόνας 
  %Μετατροπή σε double 
  I=double(I);
  
  %% NOISE
  
  fprintf(' - applying noise...\n')
  J = imnoise( I, noiseParams{:} );
  figure('Name','Noisy-Input Image');
  imagesc(J); axis image;
  colormap gray;
   saveas(gcf,fullfile(fname,'Noisy_Image.fig'));%Αποθήκευση της εικόνας με θόρυβο 
  
  %% NON LOCAL MEANS GPU IMPLEMENTATION
 %====================================
 %% PARAMETERS
  blockSize=16;
  imageSizePixel= length(I); %Διαστάσεις εικόνας
  threadsPerBlock = [blockSize blockSize];
  m = imageSizePixel;
  n = imageSizePixel;

   
  
  k = parallel.gpu.CUDAKernel( '../cuda/GPUnonLocalMean.ptx', ...
                               '../cuda/GPUnonLocalMean.cu');
  fprintf('...begin %s...\n',mfilename); 
  
  numberOfBlocks  = ceil( [m n] ./ threadsPerBlock );
  %Διαστάσεις των blocks
  k.ThreadBlockSize = threadsPerBlock;
  %Διαστάσεις του Grid
  k.GridSize        = numberOfBlocks;
    
  %Αρχικοποίηση πινάκων στην Gpu
  OutImage = zeros([m n], 'gpuArray');
  HalfPacthWidth=(patchSize-1)/2;
  NoisingImage = gpuArray(J);%Αντιγραφή της εικόνας στην gpu
 
  %Μετατροπή δεδομένων σε τύπο float
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
  saveas(gcf,fullfile(fname,'Filtered_Image.fig')); %Αποθήκευση της αποθορυβοποιημένης εικόνας 
  
  figure('Name', 'Residual');
  imagesc(OutImage-J); axis image;
  colormap gray;
  saveas(gcf,fullfile(fname,'Residual.fig'));%Αποθήκευση της εικόνας των διαφορών
  %% (END)

  fprintf('...end %s...\n',mfilename);


end

