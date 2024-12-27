import logging
import tempfile
import os
from pygerber.gerberx3.api.v2 import GerberFile, Project as GerberLayerStack
from pygerber.gerberx3.math.bounding_box import BoundingBox
from pygerber.gerberx3.math.vector_2d import Vector2D
from decimal import Decimal



logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

class GerberWrapper:
    def __init__(self):
        self.gerberStack = None
        logging.info("GerberWrapper initialized.")

    def parseGerberFile(self, file_path):
        """Parse a Gerber file and return its content."""
        try:
            with open(file_path, 'r') as f:
                content = f.read()
            gerber_file = GerberFile.from_str(content)
            logging.info(f"Successfully parsed Gerber file: {file_path}")
            return gerber_file
        except Exception as e:
            logging.error(f"Failed to parse Gerber file: {file_path}. Exception: {e}")
            raise

    def loadGerberFiles(self, gerber_file_paths):
        """Load multiple Gerber files into the layer stack."""
        try:
            loaded_files = [self.parseGerberFile(file_path) for file_path in gerber_file_paths]
            self.gerberStack = GerberLayerStack(files=loaded_files)
            logging.info("Successfully initialized GerberLayerStack with all files.")
        except Exception as e:
            logging.error(f"Failed to load Gerber files. Exception: {e}")
            raise

    def clearGerberFiles(self):
        """Clear all loaded files."""
        self.gerberStack = None
        logging.info("Cleared all loaded Gerber files.")

    def renderToPng(self, tempFilePath = None, dpmm=100):
        """Render the Gerber layer stack to a PNG image."""
        if not self.gerberStack:
            logging.warning("No layer stack initialized. Please load Gerber files first.")
            return

        try:
            parsed_stack = self.gerberStack.parse()
            logging.info("Layer stack parsed successfully.")
            #if tempFilePath is None:

            with tempfile.NamedTemporaryFile(delete=False, suffix='.png') as tmpFile:
                tempFilePath = tmpFile.name
            logging.info(f"Rendering to output path: {tempFilePath}")
            parsed_stack.render_raster(tempFilePath, dpmm=dpmm)
            logging.info(f"Successfully rendered GerberLayerStack to PNG: {tempFilePath}")
            return tempFilePath

        except Exception as e:
            logging.error(f"Failed to render Gerber layer stack. Exception: {e}")
            raise

    def processBoundingBox(self, bounding_box):
        """ Bounding box calculatations """

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
        """Get bounding box from the layer stack."""
        if not self.gerberStack:
            logging.warning("No Gerber files loaded to calculate the bounding box.")
            return None

        try:
            all_min_x = []
            all_min_y = []
            all_max_x = []
            all_max_y = []

            for gerber_file in self.gerberStack.files:
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

            # Calculate overall bounding box dimensions
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
