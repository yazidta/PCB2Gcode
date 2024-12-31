import logging
import tempfile
from pathlib import Path

from pygerber.gerberx3.api.v2 import GerberFile, Project, FileTypeEnum
from pygerber.backend.rasterized_2d.color_scheme import ColorScheme
from pygerber.gerberx3.api._v2 import ImageFormatEnum, PixelFormatEnum
from pygerber.common.rgba import RGBA


class GerberWrapper:
    """
    Wrapper class for handling Gerber files and rendering them to images.
    """

    def __init__(self):
        logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
        self.project = None
        logging.info("GerberWrapper initialized.")

    def parseGerberFile(self, file_path: str, file_type: FileTypeEnum) -> GerberFile:
        """
        Parses a single Gerber file with the specified file type.

        Args:
            file_path: Path to the Gerber file.
            file_type: Type of the Gerber file (e.g., COPPER, MASK, etc.).

        Returns:
            A parsed GerberFile instance.
        """

        try:
            gerber_file = GerberFile.from_file(file_path, file_type=file_type)
            logging.info(f"Successfully parsed Gerber file: {file_path}")
            return gerber_file
        except Exception as e:
            logging.error(f"Failed to parse Gerber file: {file_path}. Exception: {e}")
            raise

    def loadGerberFiles(self, gerber_file_paths: list):
        """
        Loads multiple Gerber files (copper, mask, silk, and edge cuts) into the project.

        Args:
            gerber_file_paths: List of file paths to the Gerber files.
        """
        try:
            if len(gerber_file_paths) != 4:
                raise ValueError("Expected exactly 4 Gerber files (copper, mask, silk, edge).")
            file_types = [FileTypeEnum.COPPER, FileTypeEnum.MASK, FileTypeEnum.SILK, FileTypeEnum.EDGE]

            self.project = Project(
                [
                    self.parseGerberFile(file_path, file_type)
                    for file_path, file_type in zip(gerber_file_paths, file_types)
                ]
            )
            logging.info("Successfully initialized GerberLayerStack with all files.")
        except Exception as e:
            logging.error(f"Failed to load Gerber files. Exception: {e}")
            raise

    def clearGerberFiles(self):
        """
        Clears the currently loaded Gerber files from memory.
        """
        self.project = None
        logging.info("Cleared all loaded Gerber files.")

    def renderToPng(self, dpmm=100):
        """
        Renders the loaded Gerber files to a PNG image, with customized colors.

        Args:
            dpmm: Dots per millimeter for rendering resolution. Defaults to 100.

        Returns:
            Path to the rendered PNG file.
        """
        if not self.project:
            logging.warning("No layer stack initialized. Please load Gerber files first.")
            return None

        try:
            with tempfile.NamedTemporaryFile(delete=False, suffix='.png') as tempFile:
                tempFilePath = tempFile.name
            logging.info(f"Rendering to output path: {tempFilePath}")

            self.project.parse().render_raster(tempFilePath, dpmm=60)
            logging.info(f"Successfully rendered GerberLayerStack to PNG: {tempFilePath}")
            return tempFilePath
        except Exception as e:
            logging.error(f"Failed to render Gerber layer stack. Exception: {e}")
            raise
    def processBoundingBox(self, bounding_box):
        try:
            min_x = bounding_box['min_x']
            min_y = bounding_box['min_y']
            max_x = bounding_box['max_x']
            max_y = bounding_box['max_y']
            width = max_x - min_x
            height = max_y - min_y
            logging.info(f"Bounding box width: {width}, height {height}")
        except KeyError as e:
            logging.error(f"Missing key in bounding box: {e}")



    def getBoundingBox(self):
        if not self.project:
            logging.warning("No Gerber files loaded to calculate the bounding box.")
            return None

        try:
            all_min_x = []
            all_min_y = []
            all_max_x = []
            all_max_y = []

            for gerber_file in self.project.files:
                parsed_file = gerber_file.parse()
                file_info = parsed_file.get_info()
                all_min_x.append(float(file_info.min_x_mm))
                all_min_y.append(float(file_info.min_y_mm))
                all_max_x.append(float(file_info.max_x_mm))
                all_max_y.append(float(file_info.max_y_mm))

            min_x = min(all_min_x)
            min_y = min(all_min_y)
            max_x = max(all_max_x)
            max_y = max(all_max_y)

            bounding_box_dict = {
                "min_x": min_x,
                "min_y": min_y,
                "max_x": max_x,
                "max_y": max_y,
            }

            self.processBoundingBox(bounding_box_dict)
            logging.info(f"Bounding box calculated: {bounding_box_dict}")
            return bounding_box_dict
        except Exception as e:
            logging.error(f"Failed to fetch bounding box. Exception: {e}")
            raise

