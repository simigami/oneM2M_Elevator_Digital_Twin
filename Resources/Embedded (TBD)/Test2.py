import os


def rename_images(folder_path):
    # 폴더 내 모든 파일을 가져옴
    for root, dirs, files in os.walk(folder_path):
        for file_name in files:
            if file_name.startswith("image") and file_name.endswith(".png"):
                # 현재 파일의 절대 경로
                file_path = os.path.join(root, file_name)

                # 이미지 파일의 숫자 추출 (예: "image1.png" -> 1)
                num = int(file_name[5:-4])

                # 새로운 파일 이름 생성
                new_file_name = f"img{num}.png"
                new_file_path = os.path.join(root, new_file_name)

                # 파일 이름 변경
                os.rename(file_path, new_file_path)
                print(f"{file_name} -> {new_file_name}")


# 사용 예시
folder_path = fr"E:\oneM2M_Docx_to_MD\spec2md\TR-0051\out"
rename_images(folder_path)
