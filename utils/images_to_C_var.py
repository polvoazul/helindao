import numpy as np
import pprint
from collections import Counter

def images_to_C_var(images):
    def img_to_C_string(img, idx): #  simple
        formatted = np.vectorize(lambda x: str(x).rjust(3))(np.array(img)).tolist()
        return f'// Image {idx}\n' + pprint.pformat(formatted, indent=2).replace('[', '{').replace(']', '}').replace("'","")
    
    def img_to_C_string(img, idx): # 8 p line
        formatted = np.vectorize(lambda x: str(x).rjust(3))(np.array(img)).tolist()
        formatted = f'  {{  // Image {idx} \n' + ', \n'.join('    {' + ', '.join('{' + ', '.join(colors) + '}' for colors in line) + '}' for line in formatted) + '}'
        return formatted
        

    SEP = ',\n'
    name = 'characters'
    out = f'''
/* generated using {scale=} {margin_x=} {margin_y=} {n_images=} {interval=} */
const unsigned char images8x8_{name}[{len(images)}][8][8][3] = {{\n{
        SEP.join(img_to_C_string(img, idx) for idx, img in enumerate(images))
    }}};'''.strip()
    assert Counter(out)['{'] == Counter(out)['}'], 'Your braces are wrong'
    return out