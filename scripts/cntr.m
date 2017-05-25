
% load the specified dataset.
dat = load(argv(){1});

% get the contour levels.
% [0.0, 0.4, 0.5, 0.6, 1.0];
lev = str2num(argv(){2});

% extract the coordinate axes.
x = unique(dat(:,2));
y = unique(dat(:,3));
z = dat(:,4);

% extract the data matrix.
zz = reshape(z, length(y), length(x));

% contour the data.
[xx, yy] = meshgrid(x, y);
C = contour(xx, yy, zz, lev);

% begin writing output.
idx = 1;
N = columns(C);
while (idx <= N)
  % insert nans between groups of points.
  if (idx > 1)
    printf('\n');
  end

  % get the current contour height and point count.
  z = C(1, idx);
  n = C(2, idx);

  % get the contour points.
  x = C(1, idx+1 : idx+n)';
  y = C(2, idx+1 : idx+n)';
  z = repmat(z, n, 1);

  % write the output values.
  for i = 1 : n
    printf('%e %e %e\n', x(i), y(i), z(i));
  end
  idx += n + 1;
end

