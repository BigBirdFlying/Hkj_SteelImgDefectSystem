import numpy as np


def flip180(arr):
    new_arr = arr.reshape(arr.size)
    new_arr = new_arr[::-1]
    new_arr = new_arr.reshape(arr.shape)
    return new_arr


def flip90_left(arr):
    new_arr = np.transpose(arr)
    new_arr = new_arr[::-1]
    return new_arr


def flip90_right(arr):
    new_arr = arr.reshape(arr.size)
    new_arr = new_arr[::-1]
    new_arr = new_arr.reshape(arr.shape)
    new_arr = np.transpose(new_arr)[::-1]
    return new_arr


def mirror(arr):
    return np.fliplr(arr)


def get_curr_steel_contour(path):
    with open(path, 'r') as f:
        info = f.read()
    contour_x = []
    contour_y = []
    info_list = info.split('\n')
    for i in range(0, len(info_list)):
        if info_list[i] != '':
            x, y = info_list[i].split('_')
            contour_x.append(int(x))
            contour_y.append(int(y))
    return contour_x, contour_y


def get_curr_steel_edge(path):
    with open(path, 'r') as f:
        info = f.read()
    edge_info = {}
    info_list = info.split('\n')
    for i in range(0, len(info_list)):
        if info_list[i] != '':
            v_pix, h_l_pix, h_r_pix, w_pix = info_list[i].split('_')
            h_pix = []
            h_pix.append(int(h_l_pix))
            h_pix.append(int(h_r_pix))
            edge_info[int(v_pix)] = h_pix
    return edge_info


def get_steel_contour_actual_coord(path):
    with open(path, 'r') as f:
        info = f.read()
    coord_x = []
    coord_y = []
    info_list = info.split('\n')
    for i in range(0, len(info_list)):
        if info_list[i] != '':
            x, y = info_list[i].split('_')
            coord_x.append(int(x))
            coord_y.append(int(y))
    coord_x.append(coord_x[0])
    coord_y.append(coord_y[0])
    return coord_x, coord_y


def get_steel_contour_actual_edge(path):
    with open(path, 'r') as f:
        info = f.read()
    edge_length = []
    edge_left = []
    edge_right = []
    edge_width = []
    edge_mark = []
    info_list = info.split('\n')
    for i in range(0, len(info_list)):
        if info_list[i] != '':
            length, left, right, width, mark = info_list[i].split('_')
            edge_length.append(int(length))
            edge_left.append(int(left))
            edge_right.append(int(right))
            edge_width.append(int(width))
            edge_mark.append(int(mark))
    return edge_length, edge_left, edge_right, edge_width, edge_mark


def get_steel_contour_actual_cutline(path):
    with open(path, 'r') as f:
        info = f.read()
    cut_top = []
    cut_bottom = []
    cut_left = []
    cut_right = []
    info_list = info.split('\n')
    for i in range(0, len(info_list)):
        if info_list[i] != '':
            top, bottom, left, right = info_list[i].split('_')
            cut_top.append(int(top))
            cut_bottom.append(int(bottom))
            cut_left.append(int(left))
            cut_right.append(int(right))
    return cut_top, cut_bottom, cut_left, cut_right


def get_defect_coord(defects_list):
    top_x = []
    top_y = []
    top_class = []
    bottom_x = []
    bottom_y = []
    for i in range(0, len(defects_list)):
        if defects_list[i][1] is True:
            class_no = defects_list[i][4]
            top_x.append(defects_list[i][3])
            top_y.append(defects_list[i][2])
        else:
            class_no = defects_list[i][4]
            bottom_x.append(defects_list[i][3])
            bottom_y.append(defects_list[i][2])


def db_binary_to_img(type_img, img_w, img_h):
    return np.array(list(type_img), np.uint8).reshape(img_w, img_h)
