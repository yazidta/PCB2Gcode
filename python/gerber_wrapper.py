import logging
from pygerber.gerberx3.api.v2 import GerberFile, Project as GerberLayerStack

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
            self.gerberStack = GerberLayerStack(loaded_files)
            logging.info("Successfully initialized GerberLayerStack with all files.")
        except Exception as e:
            logging.error(f"Failed to load Gerber files. Exception: {e}")
            raise

    def clearGerberFiles(self):
        """Clear all loaded files."""
        self.gerberStack = None
        logging.info("Cleared all loaded Gerber files.")

    def renderToPng(self, output_path, dpmm=100):
        """Render the Gerber layer stack to a PNG image."""
        if not self.gerberStack:
            logging.warning("No layer stack initialized. Please load Gerber files first.")
            return

        try:
            parsed_stack = self.gerberStack.parse()
            logging.info("Layer stack parsed successfully.")
            parsed_stack.render_raster(output_path, dpmm=dpmm)
            logging.info(f"Successfully rendered GerberLayerStack to PNG: {output_path}")
        except Exception as e:
            logging.error(f"Failed to render Gerber layer stack. Exception: {e}")
            raise
