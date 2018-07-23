function [im] = mblend(image_name1, image_name2)
% mblend merges a specific region in the first image
% to a user specified region in the second image using
% multi-band blending
%
%  image_name1: the file name of the foreground image
%  image_name2: the file name of the background image
%  im: the blending result
%
%  By Hao Jiang, Boston College, Fall 2010

im1 = imread(image_name1);
im2 = imread(image_name2);
im1 = im2double(im1);
im2 = im2double(im2);

disp('Use mouse to select a region on the first image ...');

[mk, xi, yi] = roipoly(im1);
xi = floor(xi);
yi = floor(yi);
im1 = imcrop(im1, [min(xi), min(yi), max(xi)-min(xi), max(yi)-min(yi)]);
mk = imcrop(mk, [min(xi), min(yi), max(xi)-min(xi), max(yi)-min(yi)]);

disp('Use mouse to select a region on the second image ...');

[mt, xi, yi] = roipoly(im2);
xi = floor(xi);
yi = floor(yi);
im1 = imresize(im1, [max(yi)-min(yi)+1, max(xi)-min(xi)+1]);

% Here we simply pad the background using the everage foreground color
% A better padding method should be used for better results
 
im1t = cat(3, ones(size(im2(:,:,1))) * mean(mean(im1(:,:,1))),...
              ones(size(im2(:,:,1))) * mean(mean(im1(:,:,2))),...
              ones(size(im2(:,:,1))) * mean(mean(im1(:,:,3))));    
im1t(min(yi):max(yi), min(xi):max(xi),:) = im1;

im1_size = size(im1);
im1 = im1t;  
mk = imresize(mk, [max(yi)-min(yi)+1, max(xi)-min(xi)+1], 'bilinear');
mask = zeros(size(im2));
mask(min(yi):max(yi), min(xi):max(xi),1) = mk;
mask(min(yi):max(yi), min(xi):max(xi),2) = mk;
mask(min(yi):max(yi), min(xi):max(xi),3) = mk;
imx = im1 .* mask + im2 .* (1-mask);
im1p{1} = im1;
im2p{1} = im2;
mp{1} = im2double(mask);

%M = floor(log2(max(size(im2)))); 
M = floor(log2(max(im1_size)));

for n = 2 : M
    im1p{n} = imresize(im1p{n-1}, 0.5);
    im2p{n} = imresize(im2p{n-1}, 0.5);
    mp{n} = imresize(mp{n-1}, 0.5, 'bilinear');
end

for n = 1 : M-1
    im1p{n} = im1p{n} - imresize(im1p{n+1}, [size(im1p{n},1), size(im1p{n},2)]);
    im2p{n} = im2p{n} - imresize(im2p{n+1}, [size(im2p{n},1), size(im2p{n},2)]);   
end   
 
for n = 1 : M
    imp{n} = im1p{n} .* mp{n} + im2p{n} .* (1-mp{n});
end

im = imp{M};
for n = M-1 : -1 : 1
    im = imp{n} + imresize(im, [size(imp{n},1) size(imp{n},2)]);
end

imshow(im);
