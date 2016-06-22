% Soha Rostaminia, June 3rd, 2016
% Makes Video 
interval = [1 5000];
L = interval(2) - interval(1) + 1;
numfig = 1;
Name = 'Soha';
YTeyelid = zeros(L, 1);
MovieName = sprintf('%s_eyelid_39', Name);
outputVideo = VideoWriter(fullfile('/Users/srostaminia/Desktop',MovieName));
outputVideo.FrameRate = 20; 
open(outputVideo)

prev_eyelid = [];
%Training Data
up_eyelid_thresh = 20;
pupil_diameter = 25;


for i=1:L
    Num = i + interval(1) - 1;

%     if length(num2str(Num)) == 1
%         img = sprintf('%s_00000%d.txt',Name, Num);
%     elseif length(num2str(Num)) == 2
%         img = sprintf('%s_0000%d.txt',Name ,Num);
%     elseif length(num2str(Num)) == 3
%         img = sprintf('%s_000%d.txt',Name ,Num);
%     end
 
%     img = reshape(images(Num,:), 111, 112);
%     intImage = flipud(img');

%     intImage = load(img);
%     intImage = flipud(intImage);

%     pImage = intImage(:,50:53);
%     pImage = imcrop(dImage, [50 1 9 111]);
%     iImage = sum(pImage,2);

    img = reshape(images(Num,:), 4, 112);
    intImage = fliplr(img);
    intImage = intImage - mask;
    
    [ LEyelid, pupil_index ] = eyelid_detector( intImage, prev_eyelid, pupil_diameter, up_eyelid_thresh);
    
    prev_eyelid = 112 - LEyelid;
    YTeyelid(i) = LEyelid;
    
%     %Blink Detection
%     MedFil = medfilt1(YTeyelid, 3);
%     c = [-1 -1 -1 0 1 1 1];
%     y = conv(MedFil, c, 'valid');
%     y(y<0) = 0;
%     edge_detect = y;
%     peaks = find(edge_detect(2:end-1) > edge_detect(3:end) & edge_detect(2:end-1) >= edge_detect(1:end-2) & edge_detect(2:end-1) >= 10);
%     peaks = peaks + 1;
%     num_blinks = length(peaks);

    intImage(:,(prev_eyelid-1):(prev_eyelid)) = 255;
%     intImage((prev_eyelid-1):(prev_eyelid), :) = 255;
%     intImage(:, 49:50) = 255;
%     intImage(:, 53:54) = 255;
%     text_str = cell(2,1);
%     conf_val = [Num num_blinks];
%     text_str{1} = num2str(conf_val(1));
%     text_str{2} = sprintf('# blinks = %d', conf_val(2));
%     RGB = insertText(intImage,[10 2; 40 2],text_str, 'FontSize', 10, 'BoxColor', 'red');
    RGB = intImage;
    ZerotoOne = (RGB-min(RGB(:))) / (max(RGB(:)-min(RGB(:))));
    subplot(2,1,1)
    subimage(ZerotoOne)
    subplot(2,1,2)
    plot(YTeyelid(1:i))
    plot(YTeyelid(1:i))
    ylim([0 100]);
    xlim([0 L]);
    hold on
    set(gca, 'nextplot', 'replacechildren');
    set(gcf, 'Renderer', 'zbuffer');
    fig = gcf;
    frame = getframe(fig);
%     imshow(intImage, [])
    writeVideo(outputVideo,frame)

end

close(outputVideo)
