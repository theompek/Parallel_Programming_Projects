function [ ] = pipeline_nonLocalMean(patchSize,sigma,imageName)

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
  
  %% NOISE
  
  fprintf(' - applying noise...\n')
  J = imnoise( I, noiseParams{:} );
  figure('Name','Noisy-Input Image');
  imagesc(J); axis image;
  colormap gray;
   saveas(gcf,fullfile(fname,'Noisy_Image.fig'));%Αποθήκευση της εικόνας με θόρυβο
  
  %% NON LOCAL MEANS
  %==============================
  HalfPacthWidth=(patchSize-1)/2;%Υπολογισμός του μισού μήκους των patch
  
  mex serialNonLocalMean.cpp %Δημιουργία του mex αρχείου
  
  fprintf('The algorithm Serial-Non-Local-Mean begins running...\n' );
  tic;

  If = serialNonLocalMean( J, HalfPacthWidth, sigma);
  
  fprintf('The time of execution of the program---> ' );
  toc
 %==============================
 
  %% VISUALIZE RESULT
  

  figure('Name', 'Filtered image');
  imagesc(If); axis image;
  colormap gray;
   saveas(gcf,fullfile(fname,'Filtered_Image.fig')); %Αποθήκευση της αποθορυβοποιημένης εικόνας 

 
  figure('Name', 'Residual');
  imagesc(If-J); axis image;
  colormap gray;
   saveas(gcf,fullfile(fname,'Residual.fig'));%Αποθήκευση της εικόνας των διαφορών
  %% (END)

  fprintf('...end %s...\n',mfilename);

end