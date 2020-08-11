function [ ] = gpu_Pipeline_nonLocalMean(patchSize,sigma,imageName)
    
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

   
  
  k = parallel.gpu.CUDAKernel( '../cuda/gpu_kernel.ptx', ...
                               '../cuda/gpu_kernel.cu');
  fprintf('...begin %s...\n',mfilename); 
  
  numberOfBlocks  = ceil( [m n] ./ threadsPerBlock );
  
  %Ανάλογα με τις 2 διαστάσεις των block υπολογίζουμε την τρίτη και 
  %την ενσωματώνουμε στην μεταβλητή numberOfBlocks
  DimZ=numberOfBlocks(1)*numberOfBlocks(2);
  numberOfBlocks=horzcat(numberOfBlocks,DimZ);

  %Διαστάσεις των blocks
  k.ThreadBlockSize = threadsPerBlock;
  %Διαστάσεις του Grid
  k.GridSize        = numberOfBlocks;
  
  
  %Αρχικοποίηση πινάκων στην Gpu
  OutImage = zeros([m n], 'gpuArray');
  Zi= zeros([m n], 'gpuArray');
  NoisingImage = gpuArray(J);%Αντιγραφή της εικόνας στην gpu
 
  %Μετατροπή δεδομένων σε τύπο float
  NoisingImage=single(NoisingImage); 
  OutImage=single(OutImage);
  Zi=single(Zi); 

  HalfPacthWidth=(patchSize-1)/2;
  
  fprintf('The algorithm Gpu-Non-Local-Mean begins running...\n' );
  tic;
  %Επειδή χρειαζόμαστε συγχρονισμό των δεδομένων θα υπολογίσουμε πρώτα τις τιμές των Zi για κάθε pixel
  [OutImage,Zi]=feval(k,NoisingImage,OutImage,Zi,HalfPacthWidth,imageSizePixel,sigma,1);
  
  %Περιμένουμε να υπολογιστουν τα Zi
  wait(gpuDevice);
  fprintf('To Zi ypologistike' );
  
  %Υπολογίζουμε τις νέες τιμές των pixel
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
  saveas(gcf,fullfile(fname,'Filtered_Image.fig')); %Αποθήκευση της αποθορυβοποιημένης εικόνας 
  
  figure('Name', 'Residual');
  imagesc(OutImage-J); axis image;
  colormap gray;
  saveas(gcf,fullfile(fname,'Residual.fig'));%Αποθήκευση της εικόνας των διαφορών
  %% (END)

  fprintf('...end %s...\n',mfilename);


end

