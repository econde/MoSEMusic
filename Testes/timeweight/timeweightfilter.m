a = 1.813894426370144e-04;
coef_a = [1,-a];
coef_b = [1-a];
x = [1 1 0 1 0 0 0 0];

y = filter(coef_b,coef_a,x);

imp = filter(coef_b, coef_a, x);
